#pragma once

#include "BaseInc.h"
#include "IXeInc.h"
#include "CXeInspectorPanel.h"
#include "CKEMesh.h"
#include "C3DSpotLight.h"
#include "C3DRenderEngine.h"

class CEditorForm : public CWin32App, public CForm
{
public:
	CEditorForm();
	~CEditorForm();
	void GetMeta(IMetaHelper * pHelper);

	void OnCreate();
	void OnLoaded();
	int_x OnWmMessage(uint_32 uiMessage, uint_x uiParam, int_x iParam);
	void OnRefresh(rectix rect);
	int_x OnRun();
	void LoadFromXml(const char_16 * szXmlFile);

	void Initialize3D();
	virtual void OnUpdate(int_x iPeriod);
	virtual void OnRender(int_x iPeriod);

	void ResizeFrameBuffer();

	void OnMouseIn(pointix point);
	void OnMouseDownL(pointix point);
	void OnMouseUpL(pointix point);
	void OnMouseDownR(pointix point);
	void OnMouseUpR(pointix point);
	void OnMouseMove(pointix point);
	void OnSizeChanged();

	int_x OnMenuItemSelected(IControl * pControl, int_x iItemId);
	int_x trvwScene_MouseDownR(IControl * pControl, pointix point);
	int_x trvwScene_SelectedItemChanged(IControl * pControl, TreeItemT * pItem);
	int_x pnlScene_SizeChanged(IControl * pControl);
private:
	I3DExplorer * m_pExplorer;

	C3DRenderEngine * m_pRenderEngine;
	I3DVedio * m_pVedio;
	I3DFrameBuffer * m_pFrameBuffer;
	
	I3DContext * m_pContext;
	I3DTexture * m_pBackBuffer;

	I3DScene * m_pScene;
	I3DCamera * m_pCamera;

	bool m_bInView;
	I3DScene * m_pViewScene;
	I3DCamera * m_pViewCamera;

	rectix m_rcPaint;
	IPaint * m_pPaint;

	int_64 m_iLastTime;
	int_64 m_iFpsTime;
	int_x m_iFps;

	IXeSolution * m_psln;

	CTreeView m_trvwScene;
	CPanel m_pnlScene;

	CXeInspectorPanel m_xeipMain;
	

	CKEMesh * m_pMesh;
	C3DSpotLight * m_pLight;
};

