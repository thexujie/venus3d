#pragma once

#include "BaseInc.h"

class C3DRenderEngine : public ObjectT<I3DRenderEngine>
{
public:
	C3DRenderEngine(I3DExplorer * pExplorer);
	~C3DRenderEngine();

	void Reset();
	err_t RenderBeg();
	err_t RenderEnd();

	void AddLight(I3DLight * pLight);
	void AddObject(I3DRenderObject * pObject);
	void AddRenderable(I3DPass * pPass, I3DRenderable * pRenderable);

	virtual err_t RenderTo(I3DContext * pContext, I3DTexture * pRenderTarget, rectix viewport) = 0;

protected:
	I3DExplorer * m_pExplorer;

	vector<I3DRenderObject *> m_objects;
	vector<I3DLight *> m_lights;
	linear_map<I3DPass *, vector<I3DRenderable *>> m_queue;
};
