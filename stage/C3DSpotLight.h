#pragma once

#include "BaseInc.h"

class C3DSpotLight : public ObjectT<I3DRenderObject>
{
public:
	C3DSpotLight(I3DExplorer * pExplorer);
	~C3DSpotLight();

	// range.x = angle.x range.y = angle.y range.z = distance.
	void Transform(float3 pos, float3 dir, float_32 angle, float_32 distance);

	err_t PassBeg(I3DRenderEngine * pRenderEngine, renderpass_e pass);
	err_t PassEnd(I3DRenderEngine * pRenderEngine, renderpass_e pass);
	err_t Render(I3DContext * pContext, I3DPass * pPass) const;

	void Render(I3DContext * pContext);
private:
	I3DExplorer * m_pExplorer;
	float4x4 m_transform;

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

