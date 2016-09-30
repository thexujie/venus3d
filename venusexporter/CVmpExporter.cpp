#include "stdafx.h"
#include "CVmpExporter.h"


#include "max_tools.h"
using namespace max_tools::math;

/**
* 排序，按照从大到小排序。
* Todo: 没排序完成。。。
*/
static void SortWeight(float_32 * pWeights, int_x * pIndices, int_x iLength, int_x iWeightCount)
{
	if(iWeightCount < iLength)
	{
		for(int_x cnt = 0; cnt < iLength - 1; ++cnt)
		{
			if(pWeights[cnt] < pWeights[cnt + 1])
			{
				swap(pWeights[cnt], pWeights[cnt + 1]);
				swap(pIndices[cnt], pIndices[cnt + 1]);
			}
		}

		float_32 fWeight = 0.0f;
		for(int_x cnt = 0; cnt < iWeightCount; ++cnt)
			fWeight += pWeights[cnt];

		if(!f32_equal(fWeight, 1.0f))
		{
			float_32 fRemain = 1.0f - fWeight;

			for(int_x cnt = 0; cnt < iWeightCount; ++cnt)
				pWeights[cnt] += fRemain * pWeights[cnt] / fWeight;

			for(int_x cnt = iWeightCount; cnt < iLength; ++cnt)
				pWeights[cnt] = 0.0f;
		}
	}
}

static uint_32 ColorFromPoint3(const Point3 & ptColor)
{
	uint_32 uiR = (uint_32)(ptColor.x * 0xFF);
	uint_32 uiG = (uint_32)(ptColor.y * 0xFF);
	uint_32 uiB = (uint_32)(ptColor.z * 0xFF);
	if(uiR > 0xFF) uiR = 0xFF;
	if(uiG > 0xFF) uiG = 0xFF;
	if(uiB > 0xFF) uiB = 0xFF;
	return 0xFF000000 | (uiR << 16) | (uiG << 8) | uiB;
}

static int_x GetVertexWeightCount(vertexformat_e eFormat)
{
	if(eFormat & vertexformat_wgt)
		return 4;
	else
		return 0;
}

Point3 GetVertexNormal(Mesh * pMesh, int iFace, int iVert)
{
	RVertex * pRVertex = pMesh->getRVertPtr(iVert);
	const Face & Face = pMesh->faces[iFace];
	const DWORD smGroup = Face.smGroup;
	const int normalCount = pRVertex->rFlags & NORCT_MASK;
	if(pRVertex->rFlags & SPECIFIED_NORMAL)
	{
		return pRVertex->rn.getNormal();
	}
	// ... otherwise, check for a smoothing group
	else if((normalCount > 0) && (smGroup != 0))
	{
		if(normalCount == 1)
		{
			return pRVertex->rn.getNormal();
		}
		else
		{
			for(int normalId = 0; normalId < normalCount; normalId++)
			{
				if(pRVertex->ern[normalId].getSmGroup() & smGroup)
				{
					return pRVertex->ern[normalId].getNormal();
				}
			}
			return pRVertex->rn.getNormal();
		}
	}
	else
		return pMesh->getFaceNormal(iFace);
}


void CalcTangent(Point3(&tangents)[3], Point3(&positions)[3], Point2(&texcoords)[3])
{

}

CVmpExporter::CVmpExporter(Interface * pi, ITextOutputStream * pLog, int_32 iFrame, bool bExportSelected) :
	m_pi(pi), m_pLog(pLog), m_iIndex(0), m_iFrame(iFrame), m_bExportSelected(bExportSelected)
{
}

int CVmpExporter::callback(INode * pNode)
{
	if(m_bExportSelected && !pNode->Selected())
		return TREE_CONTINUE;

	log0(L"[%02d]Name=%s.", m_iIndex, pNode->GetName());
	log0(L"[%02d]NodeId=%d.\n", m_iIndex, pNode->GetID());
	++m_iIndex;

	const ObjectState & objState = pNode->EvalWorldState(m_iFrame);
	Object * pObject = objState.obj;
	if(pObject)
	{
		Class_ID cid = pObject->ClassID();
		switch(cid.PartA())
		{
		case ClassId_Bip:
			//m_animNodes.add(pNode);
			break;
		case ClassId_Box:
		case ClassId_Sphere:
		case ClassId_Cylinder:
		case ClassId_Plane:
			//case ClassId_Pyramid:
			m_triNodes.add(pNode);
			break;
		case ClassId_TriObj:
			m_triNodes.add(pNode);
			break;
		case ClassId_EditableObj:
			m_triNodes.add(pNode);
			break;
		case ClassId_PolyObj:
			m_triNodes.add(pNode);
			break;
		case ClassId_EditablePoly:
			m_triNodes.add(pNode);
			break;
		case ClassId_IkChainObj:
			//m_animNodes.add(pNode);
			break;
		default:
			break;
		}

		/*if(pObject->IsRefTarget())
		{
		}
		else */

		//if(pObject->CanConvertToType(Class_ID_Bone))
		//{
		//	Printf(L"[%02d]HelperNode=%S\n", m_iIndex, pNode->GetName());
		//	Printf(L"[%02d]HelperNodeId=%d\n\n", m_iIndex, pNode->GetID());
		//}
		//else if(pObject->CanConvertToType(Class_ID_Bone))
		//{
		//	Printf(L"[%02d]BoneNode=%S\n", m_iIndex, pNode->GetName());
		//	Printf(L"[%02d]BoneNodeId=%d\n\n", m_iIndex, pNode->GetID());
		//}
		//switch(pObject->SuperClassID())
		//{
		//case GEOMOBJECT_CLASS_ID: // 几何体
		//	//switch(pObject->ClassID().PartA())
		//	//{
		//	//case TRIOBJ_CLASS_ID:
		//	//	m_triobjs.Add(pNode);
		//	//	break;
		//	//default:
		//	//	break;
		//	//}
		//	if(pObject->CanConvertToType(Class_ID_Triobj))
		//		m_triobjs.Add(pNode);
		//	break;
		//case SHAPE_CLASS_ID: //图形
		//	break;
		//case LIGHT_CLASS_ID: //灯光
		//	break;
		//case CAMERA_CLASS_ID: //摄影机
		//	break;
		//case HELPER_CLASS_ID: //辅助对象
		//	break;
		//default:
		//	break;
		//}
	}

	return TREE_CONTINUE;
}

#include "VmpGenerator.h"
err_t CVmpExporter::Parse(bool bLocalCoord)
{
	exporter::VmpGenerator gen;
	gen.Generate(m_triNodes);
	return err_ok;
}

#if 0
err_t CVmpExporter::Parse(bool bLocalCoord)
{
	vector<max_vert_t> tmpVerts;
	vector<UVVert> tmpTVerts;
	vector<float3> tmpNormals;
	vector<float3> tmmpTangents;

	vector<vertex_id> tmpIndices;

	vector<max_face_t> tmpFaces;
	vector<int_x> tmpMtls;

	vector<int_x> tmpWeightIndices;
	vector<float_32> tmpWeights;

	Point3 normals[3];

	// 只保留前 4 个权重。
	int_x iWeightCount = 4;

	for(int_x cnt = 0, len = m_triNodes.size(); cnt < len; ++cnt)
	{
		INode * pNode = m_triNodes[cnt];
		if(!pNode)
			continue;

		tmpVerts.clear();
		tmpTVerts.clear();
		tmpNormals.clear();
		tmpIndices.clear();
		tmpFaces.clear();
		tmpMtls.clear();

		m_info.format(L"Parsing %s...", pNode->GetName());
		m_pi->ProgressUpdate((int_32)(cnt * PROGRESS_PARSE / len), FALSE, m_info);
		int iCVertMode = pNode->GetCVertMode();
		log0(L"MeshName=%s.", pNode->GetName());
		log0(L"MeshCVertMode=%d.", iCVertMode);

		bool bDeleteMe = false;

		TriObject * pTriObject = nullptr;
		Mesh * pMesh = nullptr;

		pTriObject = GetTriObjectFromNode(pNode, bDeleteMe);
		if(pTriObject)
			pMesh = &(pTriObject->GetMesh());

		if(pMesh)
		{
			if(!pMesh->normalsBuilt)
				pMesh->buildNormals();

			Mtl * pMtl = pNode->GetMtl();
			int_32 num_sub_mtls = pMtl->NumSubMtls();
			vector<exporter::MaxMtl> mtls;

			if(num_sub_mtls > 0)
			{
				mtls.resize(num_sub_mtls);
				for(int_32 cnt = 0; cnt < num_sub_mtls; ++cnt)
					mtls[cnt].Initialize(*(pMtl->GetSubMtl(cnt)));
			}
			else
			{
				mtls.resize(1);
				mtls[0].Initialize(*pMtl);
			}

			exporter::MaxMesh mmesh;
			mmesh.Initialize(*pMesh, mtls);

			int_x iSubMtlNum = pMtl ? pMtl->NumSubMtls() : 0;

			int_x iVertNum = pMesh->getNumVerts();
			int_x itVertNum = pMesh->getNumTVerts();
			int_x iFaceNum = pMesh->getNumFaces();
			Matrix3 matr3World = pNode->GetObjectTM(m_iFrame);

			if(bLocalCoord)
			{
				AffineParts ap;
				decomp_affine(matr3World, &ap);
				Matrix3 matr = TransMatrix(-ap.t);
				matr3World *= matr;
			}

			//Matrix4F matrInit = Matrix3ToMatrix4F(matr3World);

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

			tmpVerts.resize(iVertNum);
			for(int_32 ivert = 0; ivert < iVertNum; ++ivert)
			{
				max_vert_t & mVert = tmpVerts[ivert];
				buffclr(mVert);

				Point3 vert = pMesh->getVert(ivert);
				//Point3 nor = pMesh->getNormal(ivert);
				vert = matr3World.PointTransform(vert);
				TransformPoint3(vert);

				mVert.position.set(vert.x, vert.y, vert.z);

				//计算权重。
				if(pSkinData)
				{
					int_x iNumAssigned = pSkinData->GetNumAssignedBones(ivert);

					tmpWeights.resize(iNumAssigned, 0.0f);
					tmpWeightIndices.resize(iNumAssigned, 0);

					//Todo:超出Weight的均分处理。
					if(iNumAssigned > MAX_WEIGHT_COUNT)
						return ErrorTooManyWeight;

					for(int_x ibone = 0; ibone < iNumAssigned; ++ibone)
					{
						tmpWeightIndices[ibone] = pSkinData->GetAssignedBone((int_32)ivert, (int_32)ibone);
						tmpWeights[ibone] = pSkinData->GetBoneWeight((int_32)ivert, (int_32)ibone);
					}

					if(iNumAssigned > iWeightCount)
					{
						SortWeight(tmpWeights, tmpWeightIndices, iNumAssigned, iWeightCount);
						iNumAssigned = iWeightCount;
					}

					for(int_x iwgt = 0; iwgt < iNumAssigned; ++iwgt)
					{
						int_x iIndex = TryAddAnimNode(pSkin->GetBone((int_32)tmpWeightIndices[iwgt]));
						mVert.weights[iwgt] = 10.0f * (iIndex + 1) + tmpWeights[iwgt];
					}
				}
				else if(bSelfAnim)
				{
					mVert.weights.fill(0, iWeightCount, 0.0f);
					int_x iIndex = TryAddAnimNode(pNode);
					mVert.weights[0] = 10.0f * (iIndex + 1) + 1.0f;
				}
				else
				{
					mVert.weights.fill(0, iWeightCount, 0.0f);
				}
			}

			for(int_32 itvert = 0; itvert < itVertNum; ++itvert)
			{
				UVVert tVert = pMesh->getTVert(itvert);
				TransformUV(tVert);
				tmpTVerts.add(tVert);
			}

			for(int_32 iface = 0; iface < iFaceNum; ++iface)
			{
				Face & face = pMesh->faces[iface];
				int_x mtl = (int_x)face.getMatID();
				if(mtl >= iSubMtlNum)
					mtl = 0;

				normals[0] = GetVertexNormal(pMesh, iface, face.v[0]);
				normals[1] = GetVertexNormal(pMesh, iface, face.v[1]);
				normals[2] = GetVertexNormal(pMesh, iface, face.v[2]);

				TransformPoint3(normals[0]);
				TransformPoint3(normals[1]);
				TransformPoint3(normals[2]);

				int_x iNormal0 = -1, iNormal1 = -1, iNormal2 = -1;
				iNormal0 = TryAddNormal(tmpNormals, normals[0]);
				iNormal1 = TryAddNormal(tmpNormals, normals[1]);
				iNormal2 = TryAddNormal(tmpNormals, normals[2]);

				max_face_t & mFace = tmpFaces.add();
				mFace.mtl = mtl;
				if(pMesh->tvFace)
				{
					TVFace & vFace = pMesh->tvFace[iface];
					mFace.v0 = TryAddVertex(tmpIndices, face.v[0], vFace.t[0], iNormal0);
					mFace.v1 = TryAddVertex(tmpIndices, face.v[1], vFace.t[1], iNormal1);
					mFace.v2 = TryAddVertex(tmpIndices, face.v[2], vFace.t[2], iNormal2);
				}
				else
				{
					mFace.v0 = TryAddVertex(tmpIndices, face.v[0], -1, iNormal0);
					mFace.v1 = TryAddVertex(tmpIndices, face.v[1], -1, iNormal1);
					mFace.v2 = TryAddVertex(tmpIndices, face.v[2], -1, iNormal2);
				}

				//---------------------------------------组合数据
				//记录材质ID。
				bool bFound = false;
				for(int_x mtl_cnt = 0, mtl_len = tmpMtls.size(); mtl_cnt < mtl_len; ++mtl_cnt)
				{
					if(tmpMtls[mtl_cnt] == mtl)
					{
						bFound = true;
						break;
					}
				}
				if(!bFound)
					tmpMtls.add(mtl);
			}

			int_x iVertexBase = m_vertices.size();
			int_x iVertexOffset = iVertexSum;
			int_x iVertexCount = tmpIndices.size();
			int_x iIndexBase = iVertexSum;
			int_x iIndexCount = 0;
			int_x iUnitCount = 0;

			//将这些顶点加入到顶点数组里，并保存权重。
			m_vertices.resize(iVertexBase + iVertexCount);
			for(int_x ivertex = 0; ivertex < iVertexCount; ++ivertex)
			{
				vertex_t & vertex = m_vertices[iVertexBase + ivertex];
				const vertex_id & vIndex = tmpIndices[ivertex];
				const max_vert_t & mVert = tmpVerts[vIndex.vertex];
				vertex.position = mVert.position;
				if(vIndex.texcoord >= 0)
				{
					const UVVert & tVert = tmpTVerts[vIndex.texcoord];
					vertex.texcoord.set(tVert.x, tVert.y);
				}
				else
				{
					vertex.texcoord.set(0.0f, 0.0f);
				}
				vertex.normal = tmpNormals[vIndex.normal];
				vertex.weights = mVert.weights;
			}

			int_x iSubIndexOffset = iIndexSum;
			int_x iSubIndexCount = 0;

			for(int_x mtl_cnt = 0, mtl_len = tmpMtls.size(); mtl_cnt < mtl_len; ++mtl_cnt)
			{
				iSubIndexCount = 0;

				int_x mtl = tmpMtls[mtl_cnt];

				for(int_x cnt = 0; cnt < iFaceNum; ++cnt)
				{
					const max_face_t & mFace = tmpFaces[cnt];
					if(mFace.mtl != mtl)
						continue;

					m_indices.add((uint_16)mFace.v0);
					m_indices.add((uint_16)mFace.v1);
					m_indices.add((uint_16)mFace.v2);
					iSubIndexCount += 3;
				}

				if(iSubIndexCount > 0)
				{
					vm_subset_t node;
					node.vertex_offset = (int_32)iVertexOffset;
					node.vertex_count = (int_32)iVertexCount;

					node.index_offset = (int_32)iSubIndexOffset;
					node.index_count = (int_32)iSubIndexCount;

					iSubIndexOffset += iSubIndexCount;

					m_nodes.add(node);

					iIndexCount += iSubIndexCount;
					++iUnitCount;
				}
			}

			iUnitSum += iUnitCount;
			iVertexSum += iVertexCount;
			iIndexSum += iIndexCount;

			log0(L"UnitCount=%d.", iUnitCount);
			log0(L"VertexNum=%d.", iVertexCount);
			log0(L"IndexNum=%d.", iIndexCount);
		}
		if(bDeleteMe)
			pTriObject->DeleteMe();

		log0(L"\n");
	}
	return err_ok;
}
#endif

err_t CVmpExporter::ExportMesh(IDataOutputStream * pDos, vertexformat_e eVertexFormat)
{
	m_info.format(L"Exporting logs...");
	//m_pi->ProgressUpdate((int_32)(PROGRESS_PARSE), FALSE, m_info);
	//int_x iWeightCount = GetVertexWeightCount(eVertexFormat);
	//m_pLog->WriteFormat(L"Vertex,x,y,z,nx,ny,nz,tx,ty,w0,w1,w2,w3\r\n", m_vertices.size());
	//for(int_x cnt = 0, len = m_vertices.size(); cnt < len; ++cnt)
	//{
	//	const vertex_t & vertex = m_vertices[cnt];
	//	m_pLog->WriteFormat(L"%5d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\r\n", cnt,
	//		vertex.position.x, vertex.position.y, vertex.position.z,
	//		vertex.normal.x, vertex.normal.y, vertex.normal.z,
	//		vertex.texcoord.x, vertex.texcoord.y,
	//		vertex.weights[0], vertex.weights[1], vertex.weights[2], vertex.weights[3]);
	//}
	//m_pLog->WriteFormat(L"\r\n");
	//m_pLog->WriteFormat(L"Node,VertexOffset,VertexCount,IndexBase,IndexOffset,IndexCount\r\n");
	//for(int_x inode = 0, len = m_nodes.size(); inode < len; ++inode)
	//{
	//	const vm_subset_t & node = m_nodes[inode];
	//	m_pLog->WriteFormat(L"%5d,%d,%d,%d,%d\r\n",
	//		inode,
	//		node.vertex_offset, node.vertex_count,
	//		node.index_offset, node.index_count);
	//}

	//m_info.format(L"Exporting data...");
	//vm_header_t header = {vmp_magic, vmp_version};
	//header.vertexformat = eVertexFormat;
	//header.indexformat = indexformat_16;
	//header.primitiveformat = primitiveformat_tri_list;

	//header.num_vertices = (int_32)iVertexSum;
	//header.num_indices = (int_32)iIndexSum;
	//header.num_nodes = (int_32)iUnitSum;

	//pDos->Write(&header, sizeof(vm_header_t));
	//for(int_x cnt = 0, len = m_vertices.size(); cnt < len; ++cnt)
	//{
	//	const vertex_t & vertex = m_vertices[cnt];
	//	if(eVertexFormat & vertexformat_pos)
	//		pDos->Write(&vertex.position, sizeof(float3));

	//	if(eVertexFormat & vertexformat_nor)
	//		pDos->Write(&vertex.normal, sizeof(float3));

	//	if(eVertexFormat & vertexformat_tan)
	//		pDos->Write(&vertex.tangent, sizeof(float3));

	//	if(eVertexFormat & vertexformat_tex)
	//		pDos->Write(&vertex.texcoord, sizeof(float2));

	//	if(eVertexFormat & vertexformat_wgt)
	//		pDos->Write(vertex.weights, sizeof(float4));
	//}
	//pDos->Write(m_indices, m_indices.size() * sizeof(uint_16));
	//pDos->Write(m_nodes, m_nodes.size() * sizeof(vm_subset_t));

	//m_info.format(L"Exported successed !");
	//m_pi->ProgressUpdate((int_32)(PROGRESS_ALL), FALSE, m_info);
	return err_ok;
}

void CVmpExporter::ExportAnim(IDataOutputStream * pDos, int_x iFramePerSecond)
{
	// How many Ticks per Frame.
	int_x iTicksPerFrame = GetTicksPerFrame();
	// How many Frames per Second.
	int_x iFrameRate = GetFrameRate();
	// How many Ticks per Second.
	int_x iTicksPerSecond = iFrameRate * iTicksPerFrame;

	Interval range = m_pi->GetAnimRange();
	int_x iBeg = range.Start(), iEnd = range.End();
	int_x iPeriod = iTicksPerSecond / iFramePerSecond;
	int_x iFrameCount = (iEnd - iBeg + iPeriod - 1) / iPeriod;

	for(int_x cnt = 0, len = m_animNodes.size(); cnt < len; ++cnt)
	{
		INode * pNode = m_animNodes[cnt];

		va_bone_t & bone = m_bones.add();
		Matrix3 matr3Init = pNode->GetNodeTM(m_iFrame);
		TransformMatrix3(matr3Init, bone.InitPos, bone.InitRot, bone.InitScl);

		log0(L"BoneName=%S.", pNode->GetName());

		Matrix3 matrix;
		AffineParts ap;
		iBeg = range.Start();

		while(iBeg < iEnd)
		{
			matrix = pNode->GetNodeTM((TimeValue)iBeg);
			decomp_affine(matrix, &ap);

			TransformPoint3(ap.t);
			TransformPoint3(ap.k);
			TransformQuat(ap.q);

			va_frame_t & keyFrame = m_keyframes.add();
			keyFrame.Pos.set(ap.t.x, ap.t.y, ap.t.z);
			keyFrame.Rot.set(ap.q.x, ap.q.y, ap.q.z, ap.q.w);
			keyFrame.Scl.set(ap.k.x, ap.k.y, ap.k.z);

			iBeg += iPeriod;
		}

		/**
		* 控制器种类太多，一一获取会使关键帧数据过于复杂，不易于管理。
		*/
#if 0
		int_32 iSubNum = pControl->NumSubs();
		Class_ID cid = pControl->ClassID();
		ControllerTypeE eType = GetControllerType(pControl);

		Interval valid;
		Point3 posVal;
		Quat rotVal;
		ScaleValue sclVal;
		Interval range = m_pi->GetAnimRange();
		Control * pPosController = pControl->GetPositionController();
		Control * pRotController = pControl->GetRotationController();
		Control * pSclController = pControl->GetScaleController();

		ControllerTypeE ePosType = GetControllerType(pPosController);
		ControllerTypeE eRotType = GetControllerType(pRotController);
		ControllerTypeE eSclType = GetControllerType(pSclController);
		ControllerModeE ePosMode = GetPosControllerMode(pPosController);
		ControllerModeE eRotMode = GetRotControllerMode(pRotController);
		ControllerModeE eSclMode = GetSclControllerMode(pSclController);

		Control * pPosXController = nullptr;
		Control * pPosYController = nullptr;
		Control * pPosZController = nullptr;
		int_32 iPosSubNum = pPosController->NumSubs();
		for(int_32 cnt = 0; cnt < iPosSubNum; ++cnt)
			Printf("Sub = %s : %s\n", pPosController->SubAnimName(cnt), pPosController->SubAnim(cnt)->NodeName());
		if(iPosSubNum == 3)
		{
			pPosXController = pPosController->GetXController();
			pPosYController = pPosController->GetYController();
			pPosZController = pPosController->GetZController();

			for(TimeValue beg = range.Start(), end = range.End(); beg <= end; beg += 1)
			{
				if(pPosXController->IsKeyAtTime(beg, KEYAT_POSITION) ||
					pPosYController->IsKeyAtTime(beg, KEYAT_POSITION) ||
					pPosZController->IsKeyAtTime(beg, KEYAT_POSITION))
				{
					KeyPosT & key = m_bonePos.Add();
					if(ePosMode == ControllerModeTcb)
					{
						pPosController->GetValue(beg, &rotVal, valid, CTRL_ABSOLUTE);
						TransformQuat(rotVal);
						key.Position.Set(rotVal.x, rotVal.y, rotVal.z);
						key.Time = beg;
						++bone.iPosCount;
					}
					else
					{
						pPosController->GetValue(beg, &posVal, valid, CTRL_ABSOLUTE);
						TransformPoint(posVal);
						key.Position.Set(posVal.x, posVal.y, posVal.z);
						key.Time = beg;
						++bone.iPosCount;
					}
				}
			}
		}
		else
		{
			for(TimeValue beg = range.Start(), end = range.End(); beg <= end; beg += 1)
			{
				if(pPosController->IsKeyAtTime(beg, KEYAT_POSITION))
				{
					KeyPosT & key = m_bonePos.Add();
					pPosController->GetValue(beg, &rotVal, valid, CTRL_ABSOLUTE);
					TransformQuat(rotVal);
					key.Position.Set(rotVal.x, rotVal.y, rotVal.z);
					key.Time = beg;
					++bone.iPosCount;
				}
			}
		}
		int_32 iRotSubNum = pRotController->NumSubs();
		if(iRotSubNum == 3)
		{
			Control * pRotXController = pRotController->GetXController();
			Control * pRotYController = pRotController->GetYController();
			Control * pRotZController = pRotController->GetZController();

			for(TimeValue beg = range.Start(), end = range.End(); beg <= end; beg += 1)
			{
				if(pRotXController->IsKeyAtTime(beg, KEYAT_ROTATION) ||
					pRotYController->IsKeyAtTime(beg, KEYAT_ROTATION) ||
					pRotZController->IsKeyAtTime(beg, KEYAT_ROTATION))
				{
					pRotController->GetValue(beg, &rotVal, valid, CTRL_ABSOLUTE);
					KeyRotT & keyRot = m_boneRot.Add();
					TransformQuat(rotVal);
					keyRot.Rotation.Set(rotVal.x, rotVal.y, rotVal.z, rotVal.w);
					keyRot.Time = beg;
					++bone.iRotCount;
				}
			}
		}
		else
		{
			for(TimeValue beg = range.Start(), end = range.End(); beg <= end; beg += 1)
			{
				if(pRotController->IsKeyAtTime(beg, KEYAT_ROTATION))
				{
					pRotController->GetValue(beg, &rotVal, valid, CTRL_ABSOLUTE);
					KeyRotT & keyRot = m_boneRot.Add();
					TransformQuat(rotVal);
					keyRot.Rotation.Set(rotVal.x, rotVal.y, rotVal.z, rotVal.w);
					keyRot.Time = beg;
					++bone.iRotCount;
				}
			}
		}
		int_32 iSclSubNum = pSclController->NumSubs();
		if(iSclSubNum == 0)
		{
			for(TimeValue beg = range.Start(), end = range.End(); beg <= end; beg += 1)
			{
				if(pSclController->IsKeyAtTime(beg, KEYAT_SCALE))
				{
					pSclController->GetValue(beg, &sclVal, valid, CTRL_ABSOLUTE);
					KeySclT & key = m_boneScl.Add();
					TransformPoint(sclVal.s);
					key.Scale.Set(sclVal.s.x, sclVal.s.y, sclVal.s.z);
					key.Time = beg;
					++bone.iSclCount;
				}
			}
		}
		else
			Printf(L"-------------------------------\n");

		IIKControl * pIkControl = (IIKControl*)pControl->GetInterface(0x00001020);
		IIKChainControl* pIkChainControl = (IIKChainControl*)pControl->GetInterface(0x00001030);
		Matrix3 matrix;
		AffineParts ap;
		for(TimeValue beg = range.Start(), end = range.End(); beg <= end; beg += 640)
		{
			matrix = pNode->GetNodeTM(beg);
			decomp_affine(matrix, &ap);

			TransformPoint(ap.t);
			TransformPoint(ap.k);
			TransformQuat(ap.q);

			KeyPosT & keyPos = m_bonePos.Add();
			keyPos.Position.Set(ap.t.x, ap.t.y, ap.t.z);
			keyPos.Time = beg;
			++bone.iPosCount;

			KeyRotT & keyRot = m_boneRot.Add();
			keyRot.Rotation.Set(ap.q.x, ap.q.y, ap.q.z, ap.q.w);
			keyRot.Time = beg;
			++bone.iRotCount;

			KeySclT & keyScl = m_boneScl.Add();
			keyScl.Scale.Set(ap.k.x, ap.k.y, ap.k.z);
			keyScl.Time = beg;
			++bone.iSclCount;
		}
		//#else
		TimeValue frameRange = 640;
		if(cid == IKCONTROL_CLASS_ID /*||
									 cid == IKCHAINCONTROL_CLASS_ID*/)
		{
			Matrix3 matrix;
			AffineParts ap;
			for(TimeValue beg = range.Start(), end = range.End(); beg <= end; beg += frameRange)
			{
				matrix = pNode->GetNodeTM(beg);
				decomp_affine(matrix, &ap);
				TransformPoint(ap.t);
				KeyPosT & keyPos = m_bonePos.Add();
				keyPos.Position.Set(ap.t.x, ap.t.y, ap.t.z);
				keyPos.Time = beg;
				++bone.iPosCount;
				TransformQuat(ap.q);
				KeyRotT & keyRot = m_boneRot.Add();
				keyRot.Rotation.Set(ap.q.x, ap.q.y, ap.q.z, ap.q.w);
				keyRot.Time = beg;
				++bone.iRotCount;
				TransformPoint(ap.k);
				KeySclT & keyScl = m_boneScl.Add();
				keyScl.Scale.Set(ap.k.x, ap.k.y, ap.k.z);
				keyScl.Time = beg;
				++bone.iSclCount;
			}
		}
		else if(cid == IKCHAINCONTROL_CLASS_ID)
		{
			Matrix3 matrix;
			AffineParts ap;
			for(TimeValue beg = range.Start(), end = range.End(); beg <= end; beg += frameRange)
			{
				matrix = pNode->GetNodeTM(beg);
				decomp_affine(matrix, &ap);
				TransformPoint(ap.t);
				KeyPosT & keyPos = m_bonePos.Add();
				keyPos.Position.Set(ap.t.x, ap.t.y, ap.t.z);
				keyPos.Time = beg;
				++bone.iPosCount;
				TransformQuat(ap.q);
				KeyRotT & keyRot = m_boneRot.Add();
				keyRot.Rotation.Set(ap.q.x, ap.q.y, ap.q.z, ap.q.w);
				keyRot.Time = beg;
				++bone.iRotCount;
				TransformPoint(ap.k);
				KeySclT & keyScl = m_boneScl.Add();
				keyScl.Scale.Set(ap.k.x, ap.k.y, ap.k.z);
				keyScl.Time = beg;
				++bone.iSclCount;
			}
		}
		else if(cid == Class_ID(PRS_CONTROL_CLASS_ID, 0))
		{
			//Matrix3 matrix;
			//AffineParts ap;
			//for(TimeValue beg = range.Start(), end = range.End(); beg <= end; beg += frameRange)
			//{
			//	matrix = pNode->GetNodeTM(beg);
			//	decomp_affine(matrix, &ap);
			//	TransformPoint(ap.t);
			//	KeyPosT & keyPos = m_bonePos.Add();
			//	keyPos.Position.Set(ap.t.x, ap.t.y, ap.t.z);
			//	keyPos.Time = beg;
			//	++bone.iPosCount;
			//	TransformQuat(ap.q);
			//	KeyRotT & keyRot = m_boneRot.Add();
			//	keyRot.Rotation.Set(ap.q.x, ap.q.y, ap.q.z, ap.q.w);
			//	keyRot.Time = beg;
			//	++bone.iRotCount;
			//	TransformPoint(ap.k);
			//	KeySclT & keyScl = m_boneScl.Add();
			//	keyScl.Scale.Set(ap.k.x, ap.k.y, ap.k.z);
			//	keyScl.Time = beg;
			//	++bone.iSclCount;
			//}
			//Control * pPosController = pControl->GetPositionController();
			//Control * pRotController = pControl->GetRotationController();
			//Control * pSclController = pControl->GetScaleController();

			//Interval valid;
			//Point3 posVal;
			//Quat rotVal;
			//ScaleValue sclVal;

			//for(TimeValue beg = range.Start(), end = range.End(); beg <= end; beg += 1)
			//{
			//	if(pPosController && pPosController->IsKeyAtTime(beg, KEYAT_POSITION))
			//	{
			//		pPosController->GetValue(beg, &posVal, valid, CTRL_ABSOLUTE);
			//		KeyPosT & key = m_bonePos.Add();
			//		TransformPoint(posVal);
			//		key.Position.Set(posVal.x, posVal.y, posVal.z);
			//		key.Time = beg;
			//		++bone.iPosCount;
			//	}
			//	if(pRotController && pRotController->IsKeyAtTime(beg, KEYAT_ROTATION))
			//	{
			//		pRotController->GetValue(beg, &rotVal, valid, CTRL_ABSOLUTE);
			//		KeyRotT & key = m_boneRot.Add();
			//		TransformQuat(rotVal);
			//		key.Rotation.Set(rotVal.x, rotVal.y, rotVal.z, rotVal.w);
			//		key.Time = beg;
			//		++bone.iRotCount;
			//	}
			//	if(pSclController && pSclController->IsKeyAtTime(beg, KEYAT_POSITION))
			//	{
			//		pSclController->GetValue(beg, &sclVal, valid, CTRL_ABSOLUTE);
			//		KeySclT & key = m_boneScl.Add();
			//		TransformPoint(sclVal.s);
			//		key.Scale.Set(sclVal.s.x, sclVal.s.y, sclVal.s.z);
			//		key.Time = beg;
			//		++bone.iSclCount;
			//	}
			//}
		}
		else if(cid == BIPBODY_CONTROL_CLASS_ID)
		{
			Tab<TimeValue> tabHor, tabVer, tabTur;
			IBipMaster * pBipMaster = GetBipMasterInterface(pControl);
			Control * pBipHor = pBipMaster->GetHorizontalControl();
			Control * pBipVer = pBipMaster->GetVerticalControl();
			Control * pBipTur = pBipMaster->GetTurnControl();
			pBipHor->GetKeyTimes(tabHor, range, KEYAT_POSITION | KEYAT_ROTATION);
			pBipVer->GetKeyTimes(tabVer, range, KEYAT_POSITION | KEYAT_ROTATION);
			pBipTur->GetKeyTimes(tabTur, range, KEYAT_POSITION | KEYAT_ROTATION);
		}
		else if(cid == BIPSLAVE_CONTROL_CLASS_ID)
		{
			Tab<TimeValue> tabSla;
			pControl->GetKeyTimes(tabSla, range, KEYAT_POSITION | KEYAT_ROTATION);
		}
		else {}
#endif
	}

	va_header_t header = {vme_action_magic, vme_action_version,
		(int_32)m_animNodes.size(), (int_32)iFramePerSecond, (int_32)iFrameCount};
	pDos->Write(&header, sizeof(va_header_t));
	pDos->Write(m_bones, m_bones.size() * sizeof(va_bone_t));
	pDos->Write(m_keyframes, m_keyframes.size() * sizeof(va_frame_t));
}

TriObject * CVmpExporter::GetTriObjectFromNode(INode * pNode, bool & bDeleteMe)
{
	Object * pObject = pNode->EvalWorldState(m_iFrame).obj;
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

Modifier * CVmpExporter::GetModifierFromNode(INode * pNode, const Class_ID & cid)
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

int_x CVmpExporter::TryAddAnimNode(INode * pBoneNode)
{
	if(!pBoneNode)
		return 0;

	for(int_x cnt = 0, len = m_animNodes.size(); cnt < len; ++cnt)
	{
		if(m_animNodes[cnt] == pBoneNode)
			return cnt;
	}

	int_x iIndex = m_animNodes.size();
	m_animNodes.add(pBoneNode);
	return iIndex;
}

ControllerTypeE GetControllerType(Control * pController)
{
	SClass_ID scid = pController->SuperClassID();
	switch(scid)
	{
	case CTRL_FLOAT_CLASS_ID:
		return ControllerTypeFloat;
	case CTRL_POINT3_CLASS_ID:
		return ControllerTypePoint3;
	case CTRL_MATRIX3_CLASS_ID:
		return ControllerTypeMatrix3;
	case CTRL_POSITION_CLASS_ID:
		return ControllerTypePosition;
	case CTRL_ROTATION_CLASS_ID:
		return ControllerTypeRotation;
	case CTRL_SCALE_CLASS_ID:
		return ControllerTypeScale;
	case CTRL_MORPH_CLASS_ID:
		return ControllerTypeMorph;
	default:
		return ControllerTypeInvalid;
	}
}

ControllerModeE GetPosControllerMode(Control * pController)
{
	Class_ID cid = pController->ClassID();
	if(cid == Class_ID(TCBINTERP_POSITION_CLASS_ID, 0))
		return ControllerModeTcb;
	else if(cid == Class_ID(LININTERP_POSITION_CLASS_ID, 0))
		return ControllerModeLinear;
	else if(cid == Class_ID(HYBRIDINTERP_POSITION_CLASS_ID, 0))
		return ControllerModeBezier;
	else if(cid == Class_ID(MASTERPOINTCONT_CLASS_ID, 0))
		return ControllerModeConst;
	else
		return ControllerModeInvalid;
}

ControllerModeE GetRotControllerMode(Control * pController)
{
	Class_ID cid = pController->ClassID();
	if(cid == Class_ID(TCBINTERP_ROTATION_CLASS_ID, 0))
		return ControllerModeTcb;
	else if(cid == Class_ID(LININTERP_ROTATION_CLASS_ID, 0))
		return ControllerModeLinear;
	else if(cid == Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID, 0))
		return ControllerModeBezier;
	else if(cid == Class_ID(MASTERPOINTCONT_CLASS_ID, 0))
		return ControllerModeConst;
	else
		return ControllerModeInvalid;
}

ControllerModeE GetSclControllerMode(Control * pController)
{
	Class_ID cid = pController->ClassID();
	if(cid == Class_ID(TCBINTERP_SCALE_CLASS_ID, 0))
		return ControllerModeTcb;
	else if(cid == Class_ID(LININTERP_SCALE_CLASS_ID, 0))
		return ControllerModeLinear;
	else if(cid == Class_ID(HYBRIDINTERP_SCALE_CLASS_ID, 0))
		return ControllerModeBezier;
	else if(cid == Class_ID(MASTERPOINTCONT_CLASS_ID, 0))
		return ControllerModeConst;
	else
		return ControllerModeInvalid;
}
