#pragma once

#include "BaseInc.h"

namespace dxscene
{

const int_x VertexElementMax = 32;
const int_x MAX_VERTEX_STREAMS = 16;
const int_x MAX_MESH_NAME = 100;
const int_x MAX_SUBSET_NAME = 100;
const int_x MAX_FRAME_NAME = 100;
const int_x MAX_MATERIAL_NAME = 100;
const int_x MAX_MATERIAL_PATH = MAX_PATH;
const int_x MAX_TEXTURE_NAME = MAX_PATH;

enum version_e
{
	version_101 = 101,
};

enum indextype_e
{
	indextype_16 = 0,
	indextype_32,
};

enum d3d_decl_type_e : uint_8
{
	d3d_decl_type_float1 = 0,  // 1d float expanded to (value, 0., 0., 1.)
	d3d_decl_type_float2 = 1,  // 2d float expanded to (value, value, 0., 1.)
	d3d_decl_type_float3 = 2,  // 3d float expanded to (value, value, value, 1.)
	d3d_decl_type_float4 = 3,  // 4d float
	d3d_decl_type_d3dcolor = 4,  // 4d packed unsigned bytes mapped to 0. to 1. range
							   // input is in d3dcolor format (argb) expanded to (r, g, b, a)
	d3d_decl_type_ubyte4 = 5,  // 4d unsigned byte
	d3d_decl_type_short2 = 6,  // 2d signed short expanded to (value, value, 0., 1.)
	d3d_decl_type_short4 = 7,  // 4d signed short

							 // the following types are valid only with vertex shaders >= 2.0
	d3d_decl_type_ubyte4n = 8,  // each of 4 bytes is normalized by dividing to 255.0
	d3d_decl_type_short2n = 9,  // 2d signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
	d3d_decl_type_short4n = 10,  // 4d signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
	d3d_decl_type_ushort2n = 11,  // 2d unsigned short normalized (v[0]/65535.0,v[1]/65535.0,0,1)
	d3d_decl_type_ushort4n = 12,  // 4d unsigned short normalized (v[0]/65535.0,v[1]/65535.0,v[2]/65535.0,v[3]/65535.0)
	d3d_decl_type_udec3 = 13,  // 3d unsigned 10 10 10 format expanded to (value, value, value, 1)
	d3d_decl_type_dec3n = 14,  // 3d signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1)
	d3d_decl_type_float16_2 = 15,  // two 16-bit floating point values, expanded to (value, value, 0, 1)
	d3d_decl_type_float16_4 = 16,  // four 16-bit floating point values
	d3d_decl_type_unused = 17,  // when the type field in a decl is unused.
};

enum d3d_decl_method_e : uint_8
{
	d3d_decl_method_default = 0,
	d3d_decl_method_partialu,
	d3d_decl_method_partialv,
	d3d_decl_method_crossuv,    // normal
	d3d_decl_method_uv,
	d3d_decl_method_lookup,               // lookup a displacement map
	d3d_decl_method_lookuppresampled,     // lookup a pre-sampled displacement map
};

enum d3d_decl_usage_e : uint_8
{
	d3d_decl_usage_position = 0,
	d3d_decl_usage_blendweight,   // 1
	d3d_decl_usage_blendindices,  // 2
	d3d_decl_usage_normal,        // 3
	d3d_decl_usage_psize,         // 4
	d3d_decl_usage_texcoord,      // 5
	d3d_decl_usage_tangent,       // 6
	d3d_decl_usage_binormal,      // 7
	d3d_decl_usage_tessfactor,    // 8
	d3d_decl_usage_positiont,     // 9
	d3d_decl_usage_color,         // 10
	d3d_decl_usage_fog,           // 11
	d3d_decl_usage_depth,         // 12
	d3d_decl_usage_sample,        // 13
};

struct vertex_element_t
{
	WORD    Stream;     // Stream index
	WORD    Offset;     // Offset in the stream in bytes
	d3d_decl_type_e    Type;       // Data type
	d3d_decl_method_e Method;     // Processing method
	d3d_decl_usage_e    Usage;      // Semantics
	BYTE    UsageIndex; // Semantic index
};

struct dxscene_header_t
{
	version_e version;
	bool big_endian;
	uint_64 size;
	uint_64 NonBufferDataSize;
	uint_64 size_buffer;

	//Stats
	uint_32 num_vbs;
	uint_32 num_ibs;
	uint_32 num_meshs;
	uint_32 num_subsets;
	uint_32 num_frames;
	uint_32 num_mtls;

	//Offsets to Data
	uint_64 VertexStreamHeadersOffset;
	uint_64 IndexStreamHeadersOffset;
	uint_64 MeshDataOffset;
	uint_64 SubsetDataOffset;
	uint_64 FrameDataOffset;
	uint_64 MaterialDataOffset;
};

struct vb_header_t
{
	uint_64 num_vertices;
	uint_64 SizeBytes;
	uint_64 StrideBytes;
	vertex_element_t Decl[VertexElementMax];
	uint_64 DataOffset;
};

struct ib_header_t
{
	uint_64 NumIndices;
	uint_64 SizeBytes;
	indextype_e IndexType;
	uint_64 DataOffset;
};

struct mesh_t
{
	char_8 Name[MAX_MESH_NAME];
	byte_t NumVertexBuffers;
	uint_32 VertexBuffers[MAX_VERTEX_STREAMS];
	uint_32 IndexBuffer;
	uint_32 NumSubsets;
	uint_32 NumFrameInfluences; //aka bones

	float3 BoundingBoxCenter;
	float3 BoundingBoxExtents;

	uint_64 SubsetOffset;
	uint_64 FrameInfluenceOffset;
};

struct subset_t
{
	char_8 Name[MAX_SUBSET_NAME];
	uint_32 MaterialID;
	uint_32 PrimitiveType;
	uint_64 IndexStart;
	uint_64 IndexCount;
	uint_64 VertexStart;
	uint_64 VertexCount;
};

struct frame_t
{
	char_8 Name[MAX_FRAME_NAME];
	UINT Mesh;
	UINT ParentFrame;
	UINT ChildFrame;
	UINT SiblingFrame;
	float4x4 Matrix;
	UINT AnimationDataIndex;		//Used to index which set of keyframes transforms this frame
};

struct material_t
{
	char_8 Name[MAX_MATERIAL_NAME];

	// Use MaterialInstancePath
	char_8 MaterialInstancePath[MAX_MATERIAL_PATH];

	// Or fall back to d3d8-type materials
	char_8 DiffuseTexture[MAX_TEXTURE_NAME];
	char_8 NormalTexture[MAX_TEXTURE_NAME];
	char_8 SpecularTexture[MAX_TEXTURE_NAME];

	float4 Diffuse;
	float4 Ambient;
	float4 Specular;
	float4 Emissive;
	float Power;

	UINT64 Force64_1;
	UINT64 Force64_2;
	UINT64 Force64_3;
	UINT64 Force64_4;
	UINT64 Force64_5;
	UINT64 Force64_6;
};

class V3D_API CDxSceneRenderable : public ObjectT<I3DRenderable>
{
public:
	err_t Render(I3DContext * pContext, I3DPass * pPass) const
	{
		pContext->SetVertexBuffer(vb, vertexformat_pos_tex);
		pContext->SetIndexBuffer(ib, indexformat_16);
		for(const subset_t & subset : subsets)
			pContext->DrawIndexed(0, subset.IndexStart, subset.IndexCount, primitiveformat_tri_list);
		return err_ok;
	}

public:
	mesh_t mesh;
	I3DBuffer * vb;
	I3DBuffer * ib;
	vector<subset_t> subsets;
	I3DMaterial * mtl;
};

class CDxSceneRenderObject : public ObjectT<I3DRenderObject>
{
public:
	err_t PassBeg(I3DRenderEngine * pRenderEngine, renderpass_e pass);
	err_t PassEnd(I3DRenderEngine * pRenderEngine, renderpass_e pass);

	err_t load(const char_16 * file);
	err_t load(const byte_t * buffer, int_x size);
private:
	I3DExplorer * m_pExplorer;
	vector<I3DBuffer *> m_vbs;
	vector<I3DBuffer *> m_ibs;
	vector<CDxSceneRenderable> m_renaderabls;
};

err_t load(const char_16 * file);
err_t load(const byte_t * buffer, int_x size);

}

