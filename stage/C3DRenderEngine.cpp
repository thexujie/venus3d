#include "stdafx.h"
#include "C3DRenderEngine.h"

C3DRenderEngine::C3DRenderEngine(I3DExplorer * pExplorer) :
	m_pExplorer(pExplorer)
{
	SafeAddRef(m_pExplorer);
}

C3DRenderEngine::~C3DRenderEngine()
{
	SafeRelease(m_pExplorer);
}

void C3DRenderEngine::Reset()
{
}

err_t C3DRenderEngine::RenderBeg()
{
	m_lights.clear();
	m_objects.clear();
	m_queue.clear();
	return err_ok;
}

err_t C3DRenderEngine::RenderEnd()
{
	return err_ok;
}

void C3DRenderEngine::AddLight(I3DLight * pLight)
{
	m_lights.add(pLight);
}

void C3DRenderEngine::AddObject(I3DRenderObject * pObject)
{
	m_objects.add(pObject);
}

void C3DRenderEngine::AddRenderable(I3DPass * pPass, I3DRenderable * pRenderable)
{
	vector<I3DRenderable *> & renderables = m_queue[pPass];
	renderables.add(pRenderable);
}
