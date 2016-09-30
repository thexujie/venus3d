#include "stdafx.h"
#include "CEditorForm.h"
#include "C3DRenderEngineDirect.h"

CEditorForm::CEditorForm()
	: m_iLastTime(0), m_iFpsTime(0), m_iFps(0),
	m_pPaint(nullptr),
	m_pVedio(nullptr), m_pExplorer(nullptr), m_pContext(nullptr),
	m_pFrameBuffer(nullptr), m_pBackBuffer(nullptr), m_pRenderEngine(nullptr),
	m_pScene(nullptr), m_pCamera(nullptr)
{
	//SetHostType(HostTypePopup);
	SetService(OID_I3DFileDriver, CreateFileDriver_Direct11());
	SetService(OID_I3DVedioDriver, CreateVedioDriver_Direct11());
}

CEditorForm::~CEditorForm()
{
	IWindow * pWindow = m_pHost ? (IWindow *)m_pHost->GetInterface(OID_IWindow) : nullptr;
	if(pWindow)
		pWindow->WmMessageEvent().disconnect(bind(this, &CEditorForm::OnWmMessage));

	SafeRelease(m_pRenderEngine);
	SafeRelease(m_pScene);
	SafeRelease(m_pPaint);
	SafeRelease(m_pContext);
	SafeRelease(m_pCamera);
	SafeRelease(m_pBackBuffer);
	SafeRelease(m_pFrameBuffer);
	SafeRelease(m_pExplorer);
	SafeRelease(m_pVedio);
	SetService(OID_I3DFileDriver, nullptr);
	SetService(OID_I3DVedioDriver, nullptr);
}

void CEditorForm::GetMeta(IMetaHelper * pHelper)
{
	CForm::GetMeta(pHelper);
	pHelper->help(L"pnlScene", &m_pnlScene);
}

void CEditorForm::OnCreate()
{
	CForm::OnCreate();
	IWindow * pWindow = (IWindow *)m_pHost->GetInterface(OID_IWindow);
	if(pWindow)
		pWindow->WmMessageEvent().connect(bind(this, &CEditorForm::OnWmMessage));
}

void CEditorForm::OnLoaded()
{
}

int_x CEditorForm::OnWmMessage(uint_32 uiMessage, uint_x uiParam, int_x iParam)
{
	if(!m_pHost)
		return err_nullptr;

	IWindow * pWindow = (IWindow *)m_pHost->GetInterface(OID_IWindow);
	if(!pWindow)
		return err_no_data;

	switch(uiMessage)
	{
	case WM_SYSCOMMAND:
		pWindow->OnDefault(uiMessage, uiParam, iParam);
		switch(uiParam)
		{
		case SC_MAXIMIZE:
		case SC_RESTORE:
			ResizeFrameBuffer();
			break;
		case SC_MOVE:
			break;
		default:
			break;
		}
		return 1;
	case WM_EXITSIZEMOVE:
		if(m_pFrameBuffer->GetSize() != m_rect.size)
			ResizeFrameBuffer();
		return 1;
	default:
		break;
	}
	return 0;
}

void CEditorForm::OnRefresh(rectix rect)
{
	m_rcPaint = rect;
	if(!m_pScene)
		OnRender(0);
}

int_x CEditorForm::OnRun()
{
	if(!m_pScene)
		return 0;

	int_64 iNowTime = time_ms();
	if(!m_iLastTime)
		m_iLastTime = iNowTime;
	if(!m_iFpsTime)
		m_iFpsTime = iNowTime;
	int_x iPeriod = (int_x)(iNowTime - m_iLastTime);
	if(iNowTime - m_iFpsTime >= 1000)
	{
		char_16 szFps[256] = {};
		float3 pos = m_pCamera->GetPosition();
		float3 at = m_pCamera->GetLook();
		float3 right = m_pCamera->GetRight();
		float3 up = m_pCamera->GetUp();
		textformat(szFps, 256, L"Fps = %d, pos={%.2f,%.2f,%.2f}, at={%.2f,%.2f,%.2f}, right={%.2f,%.2f,%.2f}, up={%.2f,%.2f,%.2f}",
			m_iFps, 
			pos.x, pos.y, pos.z,
			at.x, at.y, at.z,
			right.x, right.y, right.z, 
			up.x, up.y, up.z);
		SetText(szFps);
		m_iFps = 0;
		m_iFpsTime = iNowTime;
	}
	++m_iFps;

	OnUpdate(iPeriod);
	OnRender(iPeriod);
	m_iLastTime = iNowTime;
	if(::GetActiveWindow() != (HWND)GetFormId() && iPeriod < 10)
		Sleep(10 - (uint_32)iPeriod);
	return 1;
}

void CEditorForm::LoadFromXml(const char_16 * szXmlFile)
{
	IUILoader::GetInstance()->Load(szXmlFile, this);
}

#include "dxscene.h"
void CEditorForm::Initialize3D()
{
	m_pVedio = Get3DVedioDriver()->CreateVedio(VedioModeHardware, nullptr);
	m_pExplorer = new C3DExplorer(m_pVedio);
	m_pContext = m_pVedio->CreateContext();
	m_pPaint = Get2DDevice()->CreatePaint(nullptr);
	m_pScene = new C3DScene(m_pExplorer);
	m_pRenderEngine = new C3DRenderEngineDirect(m_pExplorer);

	ResizeFrameBuffer();

	m_pCamera = new C3DCameraFP();
	m_pCamera->SetVisual(1000.0f);
	m_pCamera->SetPosition(float3(0.0f, 20.0f, -20.0f));
	m_pCamera->SetLook(float3(0.0f, -1.0f, 1.0f));
	m_pCamera->SetUp(float3(0.0f, 1.0f, 0.0f));
	
	//dxscene::CDxSceneRenderObject sceneobj;
	//sceneobj.load(L"dxscene/powerplant.dxscene");

	//m_pScene->SetCamera(m_pCamera);

	object_ptr<I3DEntity> girl(new C3DEntity(m_pExplorer));
	girl->Load(L"girl.obj");
	girl->SetAction(0);
	//m_pScene->AddNode(girl);

	//object_ptr<C3DTerrain> pTerrain(new C3DTerrain(m_pExplorer));
	//pTerrain->Load(L"default.ter");
	//m_pScene->AddNode(pTerrain);
}

void CEditorForm::OnUpdate(int_x iPeriod)
{
	if(!m_pScene)
		return;

	m_pScene->Update(iPeriod);
	if(!IsActived() || !IsFocused())
		return;

	float_32 fMoveSpeed = 1.0f;
	if(IsKeyDown(KeyCodeShift))
		fMoveSpeed *= 10;
	if(IsKeyDown(KeyCodeControl))
		fMoveSpeed *= 100;
	float_32 fRotSpeed = 3.1415926f * 0.5f;
	if(IsKeyDown(KeyCodeW))
		m_pCamera->ShiftZ(iPeriod * 0.001f * fMoveSpeed);
	if(IsKeyDown(KeyCodeA))
		m_pCamera->ShiftX(-iPeriod * 0.001f * fMoveSpeed);
	if(IsKeyDown(KeyCodeS))
		m_pCamera->ShiftZ(-iPeriod * 0.001f * fMoveSpeed);
	if(IsKeyDown(KeyCodeD))
		m_pCamera->ShiftX(iPeriod * 0.001f * fMoveSpeed);
	if(IsKeyDown(KeyCodeUp))
		m_pCamera->RollX(-iPeriod * 0.001f * fRotSpeed);
	if(IsKeyDown(KeyCodeDown))
		m_pCamera->RollX(iPeriod * 0.001f * fRotSpeed);
	if(IsKeyDown(KeyCodeLeft))
		m_pCamera->RollY(-iPeriod * 0.001f * fRotSpeed);
	if(IsKeyDown(KeyCodeRight))
		m_pCamera->RollY(iPeriod * 0.001f * fRotSpeed);
}

static pointix NcToForm(IControl * pControl, pointix point)
{
	if(pControl->ToIForm())
		return point;
	else
		return NcToForm(pControl->GetParent(), point + pControl->GetPosition());
}

void CEditorForm::OnRender(int_x iPeriod)
{
	if(!m_pVedio)
		return;

	static I3DEntity * pEntity = nullptr;
	if(!pEntity)
	{
		pEntity = new C3DEntity(m_pExplorer);
		pEntity->Load(L"girl.obj");
		pEntity->SetAction(0);
	}

	rectix rcScene = m_pnlScene.GetRect();
	rcScene.position = NcToForm(&m_pnlScene, pointix());
	if(m_pCamera)
	{
		float_32 fAspect = (float_32)rcScene.w / (float_32)rcScene.h;
		float4x4 matrProj = float4x4_perspective_lh(-xm_pi / 2, fAspect, 0.1f, 1000.0f);
		m_pCamera->SetMatrixP(matrProj);
	}
	m_pCamera->SetViewPort(rectix(pointix(), rcScene.size));
	m_pContext->SetCamera(m_pCamera);

	m_pRenderEngine->RenderBeg();

	m_pRenderEngine->AddObject(pEntity);
	m_pRenderEngine->RenderTo(m_pContext, m_pBackBuffer, rcScene);
	m_pRenderEngine->RenderEnd();

	if(m_pPaint/* && m_rcPaint.is_valid()*/)
	{
		//m_pPaint->BeginPaint();
		//PreOnNcPaint(m_pPaint, rectix(pointix(), GetSize()), nullptr);
		//m_pPaint->EndPaint();
		//m_rcPaint.set_empty();
	}

	m_pFrameBuffer->Present();
}

void CEditorForm::ResizeFrameBuffer()
{
	if(!m_pVedio)
		return;

	rectix rcScene = m_pnlScene.GetRect();

	m_pPaint->ResetTarget(nullptr);

	SafeRelease(m_pBackBuffer);

	if(m_pFrameBuffer)
		m_pFrameBuffer->Resize(m_rect.w, m_rect.h);
	else
	{
		m_pFrameBuffer = m_pVedio->CreateFrameBuffer();
		m_pFrameBuffer->Create(GetFormId(), m_rect.w, m_rect.h, cmode_a8r8g8b8, 1, multisample_1);
	}
	m_pBackBuffer = m_pFrameBuffer->GetBackBuffer(0);

	I2DRTarget * p2DTarget = m_pBackBuffer->Get2DRTarget();
	m_pPaint->ResetTarget(p2DTarget);
	SafeRelease(p2DTarget);

	m_pRenderEngine->Reset();
}

static pointix old_point;

void CEditorForm::OnMouseIn(pointix point)
{
	old_point = point;
	CForm::OnMouseIn(point);
}

void CEditorForm::OnMouseDownL(pointix point)
{
	CForm::OnMouseDownL(point);
}

void CEditorForm::OnMouseUpL(pointix point)
{
	CForm::OnMouseUpL(point);
}

void CEditorForm::OnMouseDownR(pointix point)
{
	Capture(true);
	CForm::OnMouseDownR(point);
}

void CEditorForm::OnMouseUpR(pointix point)
{
	Capture(false);
	CForm::OnMouseUpR(point);
}

void CEditorForm::OnMouseMove(pointix point)
{
	pointix offset = point - old_point;
	old_point = point;
	//Printf(L"x = %d, y = %d.\n", point.X, point.Y);
	if(m_bMouseNcDownR && m_pCamera)
	{
		m_pCamera->RollX(offset.y * 0.005f);
		m_pCamera->RollY(offset.x * 0.005f);
	}
	CForm::OnMouseMove(point);
	//m_pCamera->ShiftZ(iOffZ * 0.01f);
}

void CEditorForm::OnSizeChanged()
{
	if(!IsMinSize())
	{
		CForm::OnSizeChanged();
		ResizeFrameBuffer();
		OnRender(0);
	}
}

int_x CEditorForm::pnlScene_SizeChanged(IControl * pControl)
{
	return 0;
}
