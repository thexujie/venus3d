#include "stdafx.h"
#include "C3DSpotLight.h"
#include "kemesh.h"

C3DSpotLight::C3DSpotLight(I3DExplorer * pExplorer)
{
	m_pExplorer = pExplorer;

	kemesh_t mesh;
	kemesh_load(mesh, L"spotlight.xml");

	m_posCenter = (mesh.pos_min + mesh.pos_max) * 0.5f;
	m_posExtent = mesh.pos_max - mesh.pos_min;
	m_texCenter = (mesh.tex_min + mesh.tex_max) * 0.5f;
	m_texExtent = mesh.tex_max - mesh.tex_min;

	m_pvb = m_pExplorer->GetVedio()->CreateBuffer();
	m_pib = m_pExplorer->GetVedio()->CreateBuffer();

	m_pvb->Create(sizeof(kevertex_t), mesh.vertices.size(), bufferusage_default, bufferbind_vertex, bufferaccess_none, cmode_invalid, mesh.vertices, mesh.vertices.size() * sizeof(kevertex_t));
	m_pib->Create(sizeof(uint_16), mesh.indices.size(), bufferusage_default, bufferbind_index, bufferaccess_none, cmode_invalid, mesh.indices, mesh.indices.size() * sizeof(uint_16));

	m_ptexColor = m_pExplorer->GetTexture(L"teapot/diffuse2.dds");
	SafeAddRef(m_ptexColor);
	m_ptexNormal = m_pExplorer->GetTexture(L"teapot/normal.dds");
	SafeAddRef(m_ptexNormal);

	m_indices_num = mesh.indices.size();
}

C3DSpotLight::~C3DSpotLight()
{

}

void C3DSpotLight::Transform(float3 pos, float3 dir, float_32 angle, float_32 distance)
{
	angle *= 0.5f;
	float3 scl = float3(distance * tan(angle), distance * tan(angle), distance);
	float4x4 m_pos = float4x4_translate(pos);
	float4x4 m_rot = float4x4_rotation_axis(float4((dir + float3(0, 0, 1))* 0.5f, 0.0f), xm_pi);
	float4x4 m_scl = float4x4_scale(scl);
	m_transform = m_scl * m_rot * m_pos;
	//m_transform.transform(pos, dir, scl);
}

err_t C3DSpotLight::PassBeg(I3DRenderEngine * pRenderEngine, renderpass_e pass)
{
	if(pass == renderpass_gbuffer)
	{
		I3DEffect * pEffect = m_pExplorer->GetEffect(L"static.eff");
		if(!pEffect)
			return err_no_context;
		I3DPass * pPass = pEffect->GetPass((int_x)0);
		if(!pPass)
			return err_no_context;

		//pRenderEngine->AddRenderable(pPass, this);
		return err_ok;
	}
	else
		return err_false;
}

err_t C3DSpotLight::PassEnd(I3DRenderEngine * pRenderEngine, renderpass_e pass)
{
	return err_ok;
}

err_t C3DSpotLight::Render(I3DContext * pContext, I3DPass * pPass) const
{
	I3DCamera * pCamera = pContext->GetCamera();
	if(!pCamera)
		return err_no_context;

	//float4x4 m = float4x4_transform(float3(0, 20, 0), float3(xm_pi * 0.5f, 0, 0), float3(10, 10, 10));
	float4x4 mvp = m_transform * pCamera->GetMatrixVP();
	float4x4 mv = m_transform * pCamera->GetMatrixV();

	pPass->SetVariable(L"gtmvp", &mvp, sizeof(mvp));
	pPass->SetVariable(L"gtmv", &mv, sizeof(mv));
	pPass->SetVariable(L"pos_center", &m_posCenter, sizeof(m_posCenter));
	pPass->SetVariable(L"pos_extent", &m_posExtent, sizeof(m_posExtent));
	pPass->SetVariable(L"tex_center", &m_texCenter, sizeof(m_texCenter));
	pPass->SetVariable(L"tex_extent", &m_texExtent, sizeof(m_texExtent));

	pPass->SetTexture(L"tex_color", m_ptexColor);
	pPass->SetTexture(L"tex_normal", m_ptexNormal);

	pContext->SetVertexBuffer(m_pvb, kevertex_t::FORMAT);
	pContext->SetIndexBuffer(m_pib, indexformat_16);
	pContext->DrawIndexed(0, 0, m_indices_num, primitiveformat_tri_list);
	return err_ok;
}
