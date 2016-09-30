#include "stdafx.h"
#include "C3DRenderEngineDeffered.h"

C3DRenderEngineDeffered::C3DRenderEngineDeffered(I3DExplorer * pExplorer) :
	C3DRenderEngine(pExplorer),
	m_mrt0(nullptr), m_mrt1(nullptr), m_pDepthStencil(nullptr),
	m_pvbScreen(nullptr), m_pibScreen(nullptr)
{
}

C3DRenderEngineDeffered::~C3DRenderEngineDeffered()
{
	SafeRelease(m_mrt0);
	SafeRelease(m_mrt1);
	SafeRelease(m_pvbScreen);
	SafeRelease(m_pibScreen);
	SafeRelease(m_pDepthStencil);
}

void C3DRenderEngineDeffered::Reset()
{
	SafeRelease(m_mrt0);
	SafeRelease(m_mrt1);
	SafeRelease(m_pDepthStencil);
}

err_t C3DRenderEngineDeffered::RenderBeg()
{
	m_lights.clear();
	m_objects.clear();
	m_queue.clear();
	return err_ok;
}

err_t C3DRenderEngineDeffered::RenderEnd()
{
	return err_ok;
}

err_t C3DRenderEngineDeffered::RenderTo(I3DContext * pContext, I3DTexture * pRenderTarget, rectix viewport)
{
	if(!m_mrt0)
	{
		sizeix size = pRenderTarget->GetDesc().size;
		I3DVedio * pVedio = m_pExplorer->GetVedio();
		m_mrt0 = pVedio->CreateTexture();
		m_mrt0->Create(size, cmode_a8r8g8b8, 1, bufferusage_default, bufferbind_rendertarget | bufferbind_resource,
			bufferaccess_none, multisample_1, 0);
		m_mrt1 = pVedio->CreateTexture();
		m_mrt1->Create(size, cmode_a8r8g8b8, 1, bufferusage_default, bufferbind_rendertarget | bufferbind_resource,
			bufferaccess_none, multisample_1, 0);

		m_pDepthStencil = pVedio->CreateTexture();
		m_pDepthStencil->Create(size, cmode_d24s8, 1, bufferusage_default, bufferbind_depthstencil, bufferaccess_none, multisample_1);
	}

	if(!m_pvbScreen)
	{
		I3DVedio * pVedio = m_pExplorer->GetVedio();
		struct vertex_t
		{
			float3 pos;
			float2 tex;
		};

		vertex_t vertices[4] = {
			{{-1, 1, 1}, {0, 0}},
			{{1, 1, 1}, {1, 0}},
			{{1, -1, 1}, {1, 1}},
			{{-1, -1, 1}, {0, 1}},
		};

		int_16 indices[6] = {0, 1, 2, 0, 2, 3};

		m_pvbScreen = pVedio->CreateBuffer();
		m_pibScreen = pVedio->CreateBuffer();
		m_pvbScreen->Create(sizeof(vertex_t), 4, bufferusage_default, bufferbind_vertex, bufferaccess_none, cmode_invalid,
			vertices, 4 * sizeof(vertex_t));
		m_pibScreen->Create(sizeof(int_16), 6, bufferusage_default, bufferbind_index, bufferaccess_none, cmode_invalid,
			indices, 6 * sizeof(int_16));
	}

	I3DTexture * mrt[2] = {m_mrt0, m_mrt1};
	pContext->SetRenderTarget(mrt, 2, m_pDepthStencil);
	pContext->ClearBackBuffer(0, xmf_0000);
	pContext->ClearBackBuffer(1, xmf_0000);
	pContext->ClearDepthStencil(true, 1.0f, false, 0);
	pContext->SetViewPort(rectix(pointix(), m_size));

	for(I3DRenderObject * pObject : m_objects)
		pObject->PassBeg(this, renderpass_gbuffer);

	for(pair<I3DPass *, vector<I3DRenderable *>> & ele : m_queue)
	{
		I3DPass * pPass = ele.key;
		pContext->PassBeg(pPass);
		for(I3DRenderable * pRenderable : ele.value)
			pRenderable->Render(pContext, pPass);
		pContext->PassEnd();
	}

	for(I3DRenderObject * pObject : m_objects)
		pObject->PassEnd(this, renderpass_gbuffer);

	pContext->SetRenderTarget(&pRenderTarget, 1, nullptr);
	pContext->SetViewPort(viewport);
	pContext->ClearBackBuffer(0, xmf_1111);

	I3DPass * pPass = m_pExplorer->GetEffect(L"screen.eff")->GetPass((int_x)0);
	pContext->PassBeg(pPass);
	pPass->SetTexture(L"tex_mrt0", m_mrt0);
	pPass->SetTexture(L"tex_mrt1", m_mrt1);

	pContext->SetVertexBuffer(m_pvbScreen, vertexformat_pos_tex);
	pContext->SetIndexBuffer(m_pibScreen, indexformat_16);

	pContext->DrawIndexed(0, 0, 6, primitiveformat_tri_list);

	pContext->PassEnd();
	pContext->SetRenderTarget(nullptr, 0, nullptr);

	return err_ok;
}
