#include "stdafx.h"
#include "dxscene.h"

namespace dxscene
{

err_t CDxSceneRenderObject::PassBeg(I3DRenderEngine * pRenderEngine, renderpass_e pass)
{
	return err_failed;
}

err_t CDxSceneRenderObject::PassEnd(I3DRenderEngine * pRenderEngine, renderpass_e pass)
{
	return err_failed;
}


err_t CDxSceneRenderObject::load(const char_16 * file)
{
	CFileStream fstream(file, StreamModeRead);
	byte_buffer_t buffer = fstream.ReadFile();
	return load(buffer.buffer, buffer.size);
}

err_t CDxSceneRenderObject::load(const byte_t * buffer, int_x size)
{
	//I3DVedio * pVedio = m_pExplorer->GetVedio();

	const dxscene_header_t * header = (const dxscene_header_t *)buffer;

	const vb_header_t * vb_headers = (const vb_header_t *)(buffer + header->VertexStreamHeadersOffset);
	const ib_header_t * ib_headers = (const ib_header_t *)(buffer + header->IndexStreamHeadersOffset);
	const mesh_t * meshs = (const mesh_t *)(buffer + header->MeshDataOffset);
	const subset_t * subsets = (const subset_t *)(buffer + header->SubsetDataOffset);
	const frame_t * frames = (const frame_t *)(buffer + header->FrameDataOffset);
	const material_t * materials = (const material_t *)(buffer + header->MaterialDataOffset);

	m_vbs.resize(header->num_vbs);
	m_ibs.resize(header->num_ibs);
	m_renaderabls.resize(header->num_meshs);

	const byte_t * pBufferData = buffer + header->size + header->NonBufferDataSize;
	uint_64 BufferDataStart = header->size + header->NonBufferDataSize;

	for(uint_32 ivb = 0; ivb < header->num_vbs; ++ivb)
	{
		const vb_header_t & vb_header = vb_headers[ivb];
		const byte_t * vertices = pBufferData + (vb_header.DataOffset - BufferDataStart);
		I3DBuffer *& vb = m_vbs[ivb];
		//vb = pVedio->CreateBuffer();
		//vb->Create(vb_header.StrideBytes, vb_header.num_vertices, bufferusage_default, bufferbind_vertex, bufferaccess_none, cmode_invalid, vertices, vb_header.SizeBytes);
	}

	for(uint_32 iib = 0; iib < header->num_vbs; ++iib)
	{
		const ib_header_t & ib_header = ib_headers[iib];
		const byte_t * indices = pBufferData + (ib_header.DataOffset - BufferDataStart);
		I3DBuffer *& vb = m_ibs[iib];
		//vb = pVedio->CreateBuffer();
		//switch(ib_header.IndexType)
		//{
		//case indextype_16:
		//	vb->Create(2, ib_header.NumIndices, bufferusage_default, bufferbind_vertex, bufferaccess_none, cmode_invalid, indices, ib_header.SizeBytes);
		//	break;
		//case indextype_32:
		//	vb->Create(4, ib_header.NumIndices, bufferusage_default, bufferbind_vertex, bufferaccess_none, cmode_invalid, indices, ib_header.SizeBytes);
		//	break;
		//default:
		//	break;
		//}
	}

	int_x subset_offset = 0;
	for(uint_32 imesh = 0; imesh < header->num_meshs; ++imesh)
	{
		CDxSceneRenderable & renderable = m_renaderabls[imesh];
		renderable.mesh = meshs[imesh];
		renderable.subsets.resize(renderable.mesh.NumSubsets);
		buffcpy(renderable.subsets.buffer(), subsets + subset_offset, renderable.mesh.NumSubsets);
		subset_offset += renderable.mesh.NumSubsets;
	}
	Assert(subset_offset == header->num_subsets);


	return err_ok;
}

err_t load(const char_16 * file)
{
	CFileStream fstream(file, StreamModeRead);
	byte_buffer_t buffer = fstream.ReadFile();
	return load(buffer.buffer, buffer.size);
}

err_t load(const byte_t * buffer, int_x size)
{
	const dxscene_header_t * header = (const dxscene_header_t *)buffer;

	const vb_header_t * vb_headers = (const vb_header_t *)(buffer + header->VertexStreamHeadersOffset);
	const ib_header_t * ib_headers = (const ib_header_t *)(buffer + header->IndexStreamHeadersOffset);
	const mesh_t * meshs = (const mesh_t *)(buffer + header->MeshDataOffset);
	const subset_t * subsets = (const subset_t *)(buffer + header->SubsetDataOffset);
	const frame_t * frames = (const frame_t *)(buffer + header->FrameDataOffset);
	const material_t * materials = (const material_t *)(buffer + header->MaterialDataOffset);

	return err_ok;
}

}