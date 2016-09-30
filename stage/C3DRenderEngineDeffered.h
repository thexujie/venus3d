#pragma once

#include "C3DRenderEngine.h"

class C3DRenderEngineDeffered : public C3DRenderEngine
{
public:
	C3DRenderEngineDeffered(I3DExplorer * pExplorer);
	~C3DRenderEngineDeffered();

	void Reset();
	err_t RenderBeg();
	err_t RenderEnd();
	err_t RenderTo(I3DContext * pContext, I3DTexture * pRenderTarget, rectix viewport);

private:
	sizeix m_size;
	I3DTexture * m_mrt0;
	I3DTexture * m_mrt1;

	I3DBuffer * m_pvbScreen;
	I3DBuffer * m_pibScreen;

	I3DTexture * m_pDepthStencil;

	vector<I3DRenderObject *> m_objects;
	vector<I3DLight *> m_lights;
	linear_map<I3DPass *, vector<I3DRenderable *>> m_queue;
};
