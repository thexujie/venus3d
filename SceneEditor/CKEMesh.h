#pragma once

#include "BaseInc.h"

class CKEMesh : public ObjectT<I3DRenderObject>
{
public:
	CKEMesh(I3DExplorer * pExplorer);
	~CKEMesh();

	err_t Load(const char_16 * szFile);

	err_t PassBeg(I3DRenderEngine * pRenderEngine, renderpass_e pass);
	err_t PassEnd(I3DRenderEngine * pRenderEngine, renderpass_e pass);
	err_t Render(I3DContext * pContext, I3DPass * pPass) const;
protected:
	I3DExplorer * m_pExplorer;

	vertexformat_e m_vf;
	indexformat_e m_if;
	I3DBuffer * m_pvb;
	I3DBuffer * m_pib;
	I3DTexture * m_ptexColor;
	I3DTexture * m_ptexNormal;
	int_x m_indices_num;
	int_x m_vertices_num;

	float3 m_posCenter;
	float3 m_posExtent;
	float2 m_texCenter;
	float2 m_texExtent;
};