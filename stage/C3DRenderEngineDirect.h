#pragma once

#include "C3DRenderEngine.h"

class C3DRenderEngineDirect : public C3DRenderEngine
{
public:
	C3DRenderEngineDirect(I3DExplorer * pExplorer);
	~C3DRenderEngineDirect();

	void Reset();
	err_t RenderBeg();
	err_t RenderEnd();

	err_t RenderTo(I3DContext * pContext, I3DTexture * pRenderTarget, rectix viewport);

private:
	I3DTexture * m_pDepthStencil;
};
