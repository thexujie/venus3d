#pragma once

#include "BaseInc.h"

class C3DRenderEngine : public ObjectT<I3DRenderEngine>
{
public:
	C3DRenderEngine(I3DExplorer * pExplorer);
	~C3DRenderEngine();

	void Reset(sizeix size);
	err_t RenderBeg();
	err_t RenderEnd();

	void AddLight(I3DLight * pLight);
	void AddObject(I3DRenderObject * pObject);

	void AddRenderable(I3DPass * pPass, I3DRenderObject * pRenderable);

	err_t RenderTo(I3DContext * pContext, I3DTexture * pRenderTarget, rectix viewport);

private:
	I3DExplorer * m_pExplorer;
	sizeix m_size;
	I3DTexture * m_mrt0;
	I3DTexture * m_mrt1;

	I3DBuffer * m_pvbScreen;
	I3DBuffer * m_pibScreen;

	I3DTexture * m_pDepthStencil;

	vector<I3DRenderObject *> m_objects;
	vector<I3DLight *> m_lights;
	linear_map<I3DPass *, vector<I3DRenderObject *>> m_queue;
};
