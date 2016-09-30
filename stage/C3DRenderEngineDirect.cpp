#include "stdafx.h"
#include "C3DRenderEngineDirect.h"

C3DRenderEngineDirect::C3DRenderEngineDirect(I3DExplorer * pExplorer) :
	C3DRenderEngine(pExplorer),
	m_pDepthStencil(nullptr)
{
}

C3DRenderEngineDirect::~C3DRenderEngineDirect()
{
	SafeRelease(m_pDepthStencil);
}

void C3DRenderEngineDirect::Reset()
{
	SafeRelease(m_pDepthStencil);
}

err_t C3DRenderEngineDirect::RenderBeg()
{
	m_lights.clear();
	m_objects.clear();
	m_queue.clear();
	return err_ok;
}

err_t C3DRenderEngineDirect::RenderEnd()
{
	return err_ok;
}

err_t C3DRenderEngineDirect::RenderTo(I3DContext * pContext, I3DTexture * pRenderTarget, rectix viewport)
{
	pContext->SetRenderTarget(&pRenderTarget, 1, m_pDepthStencil);
	pContext->ClearBackBuffer(0, xmf_1111);
	pContext->ClearDepthStencil(true, 1.0f, false, 0);
	pContext->SetViewPort(viewport);

	for(I3DRenderObject * pObject : m_objects)
		pObject->PassBeg(this, renderpass_normal);

	for(pair<I3DPass *, vector<I3DRenderable *>> & ele : m_queue)
	{
		I3DPass * pPass = ele.key;
		pContext->PassBeg(pPass);
		for(I3DRenderable * pRenderable : ele.value)
			pRenderable->Render(pContext, pPass);
		pContext->PassEnd();
	}

	for(I3DRenderObject * pObject : m_objects)
		pObject->PassEnd(this, renderpass_normal);

	pContext->SetRenderTarget(nullptr, 0, nullptr);

	return err_ok;
}
