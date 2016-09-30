#include "stdafx.h"
#include "VmpGenerator.h"
#include "max_tools.h"

using namespace max_tools::math;

namespace exporter
{
const uint_32 SM_GROUP_NULL = 0;

struct vindex_t
{
	int_x position;
	int_x normal;
	int_x tangent;
	int_x texcoord;

	bool operator==(const vindex_t & another) const
	{
		return position == another.position &&
			normal == another.normal &&
			tangent == another.tangent &&
			texcoord == another.texcoord;
	}
	bool operator != (const vindex_t & another) const { return !operator==(another); }
};

float3 compute_tangent(const float3 & point0, const float3 & point1, const float3 & point2,
	const float2 & tex0, const float2 & tex1, const float2 & tex2)
{
	float3 v1v0 = point1 - point0;
	float3 v2v0 = point2 - point0;

	float s1 = tex1.x - tex0.x;
	float t1 = tex1.y - tex0.y;

	float s2 = tex2.x - tex0.x;
	float t2 = tex2.y - tex0.y;

	float denominator = s1 * t2 - s2 * t1;
	if(venus::abs(denominator) < std::numeric_limits<float>::epsilon())
	{
		return float3(1, 0, 0);
	}
	else
	{
		return (t2 * v1v0 - t1 * v2v0) / denominator;
	}
}

static int_x add_vindex(vector<vindex_t> & vindices,
	const vindex_t & index)
{
	for(int_x cnt = 0; cnt < vindices.size(); ++cnt)
	{
		if(vindices[cnt] == index)
			return cnt;
	}

	vindices.add(index);
	return vindices.size() - 1;
}

static int_x TryAddFloat2(vector<float2> & points, const float2 & value)
{
	for(int_x cnt = 0, len = points.size(); cnt < len; ++cnt)
	{
		const float2 & point = points[cnt];
		if((fabs(point.x - value.x) < F32_EPSILON) &&
			(fabs(point.y - value.y) < F32_EPSILON))
			return cnt;
	}

	int_x iIndex = points.size();
	points.add(value);
	return iIndex;
}

static int_x TryAddFloat3(vector<float3> & float3s, const float3 & value)
{
	for(int_x cnt = 0, len = float3s.size(); cnt < len; ++cnt)
	{
		const float3 & temp = float3s[cnt];
		if(temp == value)
			return cnt;
	}

	int_x iIndex = float3s.size();
	float3s.add(value);
	return iIndex;
}

err_t MaxMtl::Initialize(Mtl & mtl)
{
	if(&mtl == nullptr)
		return err_nullptr;

	int_32 num_mtls = mtl.NumSubMtls();
	if(num_mtls > 0)
		return err_invalidarg;

	pmtl = &mtl;
	ambient = cv_color3(mtl.GetAmbient());
	diffuse = cv_color3(mtl.GetDiffuse());
	specular = cv_color3(mtl.GetSpecular());

	if(mtl.GetSelfIllumColorOn())
		emit = cv_color3(mtl.GetSelfIllumColor());
	else
		emit = cv_color3(mtl.GetDiffuse() * mtl.GetSelfIllum());
	opacity = 1.0f - mtl.GetXParency();
	specular_level = mtl.GetShinStr();
	shininess = mtl.GetShininess() * 100;

	int_32 num_texmaps = mtl.NumSubTexmaps();
	for(int_32 itexmap = 0; itexmap < num_texmaps; ++itexmap)
	{
		if(!mtl.SubTexmapOn(itexmap))
			continue;

		maxtexmap_t & texmap = texmaps.add();
		texmap.type = (texture_type_e)itexmap;
		texmap.type_name = mtl.GetSubTexmapSlotName(itexmap).data();

		Texmap * _texmap = mtl.GetSubTexmap(itexmap);
		if(_texmap && (Class_ID(BMTEX_CLASS_ID, 0) == _texmap->ClassID()))
		{
			BitmapTex * bitmap_tex = static_cast<BitmapTex*>(_texmap);
			texmap.mapname = bitmap_tex->GetMapName();
			if(!texmap.mapname.is_empty())
			{
				texmap.channel = bitmap_tex->GetMapChannel();
				_texmap->GetUVTransform(texmap.trans);
				UVGen* uv_gen = _texmap->GetTheUVGen();
				if(!uv_gen)
					texmap.uvwindex.set(0, 1, 2);
				else
				{
					int axis = uv_gen->GetAxis();
					switch(axis)
					{
					case AXIS_UV:
						texmap.uvwindex.set(0, 1, 2);
						break;

					case AXIS_VW:
						texmap.uvwindex.set(1, 2, 0);
						break;

					case AXIS_WU:
						texmap.uvwindex.set(2, 0, 1);
						break;
					}
				}
			}
		}
	}
	return err_ok;
}

int_x MaxMtl::GetDiffuseTexChannel() const
{
	for(const maxtexmap_t & texmap : texmaps)
	{
		if(texmap.type == texture_type_diffuse)
			return texmap.channel;
	}
	return 1;
}

const maxtexmap_t & MaxMtl::FindTexmap(texture_type_e type) const
{
	for(const maxtexmap_t & texmap : texmaps)
	{
		if(texmap.type == type)
			return texmap;
	}
	throw exp_out_of_bound();
}

err_t VmpGenerator::Generate(const vector<INode *> & nodes)
{
	for(int_x cnt = 0; cnt < nodes.size(); ++cnt)
	{
		GenerateNode(nodes[cnt]);
	}
	return err_ok;
}

static TriObject * GetTriObjectFromNode(INode * pNode, bool & bDeleteMe, TimeValue time = 0)
{
	Object * pObject = pNode->EvalWorldState(time).obj;
	if(pObject->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
	{
		TriObject *pTriObject = (TriObject *)pObject->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0));
		bDeleteMe = pTriObject != pObject;
		return pTriObject;
	}
	else
	{
		bDeleteMe = false;
		return nullptr;
	}
}

static Modifier * GetModifierFromNode(INode * pNode, const Class_ID & cid)
{
	if(!pNode)
		return nullptr;

	Object * pObject = pNode->GetObjectRef();
	while(pObject && pObject->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject * pDerivedObject = (IDerivedObject *)pObject;
		int_32 iIndex = 0;
		for(int_32 cnt = 0, len = pDerivedObject->NumModifiers(); cnt < len; ++cnt)
		{
			Modifier * pModifier = pDerivedObject->GetModifier(cnt);
			log0(L"ModifierName=%S.", pModifier->GetName());
			if(pModifier->ClassID() == cid)
				return pModifier;
		}
		pObject = pDerivedObject->GetObjRef();
	}


	return nullptr;
}

err_t VmpGenerator::GenerateNode(INode * pNode)
{
	bool bDeleteMe = false;

	// 得到 mesh.
	TriObject * pTriObject = nullptr;
	Mesh * pMesh = nullptr;
	pTriObject = GetTriObjectFromNode(pNode, bDeleteMe);
	if(pTriObject)
		pMesh = &(pTriObject->GetMesh());

	if(!pMesh)
		return err_ok;

	Mesh & mesh = *pMesh;

	// 得到 ISkinData
	Modifier * pModifier = GetModifierFromNode(pNode, Class_ID_Skin);
	ISkin * pSkin = nullptr;
	ISkinContextData * pSkinData = nullptr;

	if(pModifier)
		pSkin = (ISkin *)pModifier->GetInterface(I_SKIN);
	if(pSkin)
		pSkinData = pSkin->GetContextInterface(pNode);

	bool bSelfAnim = false;
	if(!pSkinData)
	{
		Control * pControl = pNode->GetTMController();
		Control * pPosController = pControl ? pControl->GetPositionController() : nullptr;
		Control * pRotController = pControl ? pControl->GetRotationController() : nullptr;
		Control * pSclController = pControl ? pControl->GetScaleController() : nullptr;
		if(pControl &&
			((pPosController && pPosController->IsAnimated()) ||
			(pRotController && pRotController->IsAnimated()) ||
				(pSclController && pSclController->IsAnimated())))
			bSelfAnim = true;
		else
			bSelfAnim = false;
	}

	struct faceindex_t
	{
		int_x face;

		int_x normal;
		int_x tangent;

		int_x position[3];
		int_x texcoord[3];
	};

	vector<vindex_t> vindices;

	vector<position_t> all_positions;
	vector<float3> all_normals;

	vector<float2> all_texcoords;
	vector<float3> all_tangents;

	vector<bonebind_t> all_bonebinds;

	linear_map<mtlid_t, linear_map<smid_t, vector<faceindex_t>>> all_faces;

	// 所有顶点
	int_32 num_vertices = mesh.getNumVerts();
	all_positions.resize(num_vertices);
	for(int ivertex = 0; ivertex < num_vertices; ++ivertex)
	{
		position_t & position = all_positions[ivertex];
		position.point = cv_point3(mesh.getVert(ivertex));

		//计算权重。
		if(pSkinData)
		{
			int_x iNumAssigned = pSkinData->GetNumAssignedBones(ivertex);

			all_bonebinds.resize(iNumAssigned);

			//Todo:超出Weight的均分处理。
			if(iNumAssigned > WEIGHT_MAX)
				return err_bad_data;

			for(int_x ibone = 0; ibone < iNumAssigned; ++ibone)
			{
				all_bonebinds[ibone].index = pSkinData->GetAssignedBone((int_32)ivertex, (int_32)ibone);
				all_bonebinds[ibone].weight = pSkinData->GetBoneWeight((int_32)ivertex, (int_32)ibone);
			}

			for(int_x ibone = 0; ibone < iNumAssigned; ++ibone)
			{
				int_x iIndex = AddBone(pSkin->GetBone((int_32)all_bonebinds[ibone].index));
				position.weights[ibone] = 10.0f * (iIndex + 1) + all_bonebinds[ibone].weight;
			}
		}
		else if(bSelfAnim)
		{
			position.weights.fill(0.0f);
			int_x iIndex = AddBone(pNode);
			position.weights[0] = 10.0f * (iIndex + 1) + 1.0f;
		}
		else
		{
			position.weights.fill(0.0f);
		}
	}

	Mtl * pNodeMtl = pNode->GetMtl();
	// 所有面，依照材质、平滑组分组。
	int_x num_faces = mesh.getNumFaces();
	for(int_32 iface = 0; iface < num_faces; ++iface)
	{
		Face & maxface = mesh.faces[iface];
		MtlID mtlidx = mesh.getFaceMtlIndex(iface);
		Mtl * pMtl = nullptr;
		if(pNodeMtl && pNodeMtl->IsMultiMtl())
			pMtl = pNodeMtl->GetSubMtl(mtlidx);
		else
			pMtl = pNodeMtl;
		mtlid_t mtlid = AddMtl(*pMtl);

		linear_map<smid_t, vector<faceindex_t>> & mtl_faces = all_faces[mtlid];

		float3 normal = cv_point3(mesh.getFaceNormal(iface));
		int_x inormal = TryAddFloat3(all_normals, normal);

		uint_32 sm_groups = maxface.getSmGroup();
		if(sm_groups)
		{
			for(int_x isg = 0; isg < 32; ++isg)
			{
				if(bittest(sm_groups, isg))
				{
					faceindex_t & face = mtl_faces[isg].add();
					face.face = iface;
					face.normal = inormal;
					for(int_x ivert = 0; ivert < 3; ++ivert)
						face.position[ivert] = maxface.v[2 - ivert];
				}
			}
		}
		else
		{
			faceindex_t & face = mtl_faces[SM_GROUP_NULL].add();
			face.face = iface;
			face.normal = inormal;
			for(int_x ivert = 0; ivert < 3; ++ivert)
				face.position[ivert] = maxface.v[2 - ivert];
		}
	}

	for(int_x imtl = 0; imtl < all_faces.size(); ++imtl)
	{
		all_texcoords.clear();
		all_tangents.clear();

		mtlid_t mtlid = all_faces.at(imtl).key;
		const MaxMtl & mtl = mtls[mtlid];
		int_32 channel = (int_32)mtl.GetDiffuseTexChannel();

		linear_map<smid_t, vector<faceindex_t>> & mtl_faces = all_faces.at(imtl).value;

		node_t & node = nodes.add();
		node.vertex_offset = vertices.size();
		node.vertex_count = 0;
		node.index_offset = indices.size();
		node.index_count = 0;

		for(int_x isg = 0; isg < mtl_faces.size(); ++isg)
		{
			smid_t smid = mtl_faces.at(isg).key;
			vector<faceindex_t> & faces = mtl_faces.at(isg).value;
			if(!mesh.mapSupport(channel))
				continue;

			int_32 num_map_verts = mesh.getNumMapVerts(channel);
			if(num_map_verts <= 0)
				continue;

			const maxtexmap_t & texmap = mtl.FindTexmap(texture_type_diffuse);
			UVVert * uv_verts = mesh.mapVerts(channel);
			TVFace * tv_faces = mesh.mapFaces(channel);

			for(int_x iface = 0; iface < faces.size(); ++iface)
			{
				faceindex_t & face = faces[iface];
				TVFace & tv_face = tv_faces[face.face];
				for(int_x ivert = 2; ivert >= 0; --ivert)
				{
					int_32 ti = tv_face.t[ivert];
					const UVVert & uv_vert = uv_verts[ti];

					Point3 uvw;
					if(isnan(uv_verts[ti].x) || isnan(uv_verts[ti].y) || isnan(uv_verts[ti].z))
						uvw = Point3(0, 0, 0);
					else
						uvw = uv_verts[ti] * texmap.trans;

					float2 texcoord(uvw[texmap.uvwindex.x], uvw[texmap.uvwindex.y]);
					face.texcoord[ivert] = TryAddFloat2(all_texcoords, texcoord);
				}
				float3 tangent = compute_tangent(
					all_positions[face.position[0]].point, all_positions[face.position[1]].point, all_positions[face.position[2]].point,
					all_texcoords[face.texcoord[0]], all_texcoords[face.texcoord[1]], all_texcoords[face.texcoord[2]]
				);
				face.tangent = TryAddFloat3(all_tangents, tangent);

				// 生成 tri indices.
				for(int_x ivert = 0; ivert < 3; ++ivert)
				{
					int_x index = add_vindex(vindices, {face.position[ivert], face.normal, face.tangent, face.texcoord[ivert]});
					indices.add((int_16)index);
				}
				node.index_count += 3;
			}
		}

		node.vertex_count = vindices.size();
	}

	// 生成 vertex.
	for(int_x cnt = 0; cnt < vindices.size(); ++cnt)
	{
		const vindex_t & vindex = vindices[cnt];
		vertex_t & vertex = vertices.add();
		vertex.position = all_positions[vindex.position];
		vertex.normal = all_normals[vindex.normal];
		vertex.tangent = all_tangents[vindex.tangent];
		vertex.texcoord = all_texcoords[vindex.texcoord];
	}
	return err_ok;
}

mtlid_t VmpGenerator::AddMtl(Mtl & mtl)
{
	for(int_x cnt = 0; cnt < mtls.size(); ++cnt)
	{
		if(mtls[cnt].pmtl == &mtl)
			return cnt;
	}

	MaxMtl & maxmtl = mtls.add();
	maxmtl.Initialize(mtl);
	return mtls.size() - 1;
}

int_x VmpGenerator::AddBone(INode * pBone)
{
	for(int_x cnt = 0; cnt < bones.size(); ++cnt)
	{
		if(bones[cnt] == pBone)
			return cnt;
	}
	bones.add(pBone);
	return bones.size() - 1;
}

}

