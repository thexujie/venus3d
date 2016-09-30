#include "stdafx.h"
#include "C3DApp.h"
#include <process.h>
#include <locale.h>

static unsigned __stdcall Run3DAppThreadFun(void * pParam);

C3DApp::C3DApp(HWND hWnd) :
m_pApp(nullptr), 
m_pScene(nullptr), m_pMouse(nullptr), m_pKeybd(nullptr),
m_iThread(0), m_uiThreadId(0), m_hWnd(hWnd),
m_pFileDriver(nullptr), m_pVedioDriver(nullptr), m_pInputDriver(nullptr), m_pScriptDriver(nullptr),
m_pVedio(nullptr),
m_pAdapter(nullptr), m_pOutput(nullptr),
m_pFrameBuffer(nullptr), m_pRenderTarget(nullptr),
m_pExplorer(nullptr),
m_pCamera(nullptr)
{
}

C3DApp::~C3DApp()
{
}

I3DFileDriver * C3DApp::GetFileDriver() const
{
	return m_pFileDriver;
}

I3DVedioDriver * C3DApp::GetVedioDriver() const
{
	return m_pVedioDriver;
}

I3DAudioDriver * C3DApp::GetAudioDriver() const
{
	return nullptr;
}

I3DInputDriver * C3DApp::GetInputDriver() const
{
	return m_pInputDriver;
}

I3DExplorer * C3DApp::CreateExplorer(I3DVedio * pVedio)
{
	return new C3DExplorer(this, pVedio);
}

I3DScene * C3DApp::CreateScene(I3DExplorer * pExplorer)
{
	return new C3DScene(this, pExplorer);
}

err_t C3DApp::Beg3D()
{
	m_iThread = (int_x)_beginthreadex(nullptr, 0, Run3DAppThreadFun, (void *)this, 0, &m_uiThreadId);
	return err_ok;
}

err_t C3DApp::End3D()
{
	if(m_iThread)
	{
		::PostThreadMessageW(m_uiThreadId, WM_QUIT, 0, 0);
		HANDLE hThread = (HANDLE)m_iThread;
		DWORD dwTime = 3000;
#ifdef _DEBUG
		// hit a breakpoint on debugging.
		dwTime = INFINITE;
#endif
		DWORD dwWait = ::WaitForSingleObject(hThread, dwTime);
		if(dwWait != WAIT_OBJECT_0)
		{
			TerminateThread(hThread, 1);
		}
		CloseHandle(hThread);
		m_iThread = 0;
		m_uiThreadId = 0;
	}
	SafeDelete(m_pApp);
	return err_ok;
}

err_t C3DApp::Post(uint_32 uiMessage, uint_x uiParam, int_x iParam)
{
	if(!m_uiThreadId)
		return err_not_initialized;
	::PostThreadMessageW(m_uiThreadId, uiMessage, uiParam, iParam);
	return err_ok;
}

int_x C3DApp::Run()
{
	setlocale(0, "");
	CWin32App app;

	//I2DDevice * pRenderService = new C2DDeviceD2D();
	//I2DDevice * pRenderService = new C2DDeviceGdip();
	//I2DDevice * pRenderService = new C2DDeviceGdi();

	//SetService(OID_IRenderService, pRenderService);
	//Set2DDevice(pRenderService);
	//SafeRelease(pRenderService);

	err_t err = OnInitialize();
	if(err)
		return err;
	int_64 iLastTime = time_ms();
	int_64 iFpsTime = time_ms();
	int_x iFps = 0;
	while(true)
	{
		int_64 iNowTime = time_ms();
		int_x iPeriod = (int_x)(iNowTime - iLastTime);
		if(iNowTime - iFpsTime >= 1000)
		{
			char_16 szFps[64] = {};
			textformat(szFps, 64, L"Fps = %d", iFps);
			::SetWindowTextW(m_hWnd, szFps);
			iFps = 0;
			iFpsTime = iNowTime;
		}
		++iFps;
		err = OnIdle(iPeriod);
		if(err != err_ok)
			break;

		OnUpdate(iPeriod);
		OnRender(iPeriod);
		iLastTime = iNowTime;
		if(::GetActiveWindow() != m_hWnd && iPeriod < 10)
			Sleep(10 - (uint_32)iPeriod);
	}

	OnUninitialize();
	return (int_x)err;
}

err_t C3DApp::OnInitialize()
{
	//m_pPlugin = venusclr::GetPlugin();
	//m_pPlugin->Load(L"Plugin_Test.dll");

	m_rect.size = Window::GetClient(m_hWnd).size;

	m_pFileDriver = CreateFileDriver_Direct11();
	m_pVedioDriver = CreateVedioDriver_Direct11();
	m_pInputDriver = CreateInputDriver_Direct11();

	//m_pPlugin->OnAppInitialize(this);
	//return err_ok;

	m_pAdapter = m_pVedioDriver->EnumAdapter(0);
	if(!m_pAdapter)
	{
		log2(L"EnumAdapter failed!\n");
		SafeRelease(m_pVedioDriver);
		return 1;
	}

	m_pOutput = m_pAdapter->EnumOutput(0);

	m_pVedio = m_pVedioDriver->CreateVedio(VedioModeDefalult, m_pAdapter);
	if(!m_pVedio)
	{
		log2(L"CreateDevice failed!\n");
		return 1;
	}
	m_pExplorer = new C3DExplorer(this, m_pVedio);
	// frame buffer
	m_pFrameBuffer = m_pVedio->CreateFrameBuffer();
	m_pFrameBuffer->Create((int_x)m_hWnd, m_rect.w, m_rect.h, cmode_a8r8g8b8, 1, 1, 0);

	m_pRenderTarget = m_pVedio->CreateRenderTarget();

	ResizeFrameBuffer(m_rect.w, m_rect.h);



	float_32 fAspect = (float_32)m_rect.w / (float_32)m_rect.h;
	float4x4 matrProj = float4x4_perspective_lh(xm_pi / 2, fAspect, 0.1f, 1000.0f);

	m_pCamera = new C3DCameraFP();
	m_pCamera->SetMatrixP(matrProj);
	m_pCamera->SetVisual(1000.0f);
	m_pCamera->SetPosition(float3(0.0f, 20.0f, -20.0f));
	m_pCamera->SetLook(float3(0.0f, -1.0f, 1.0f));
	m_pCamera->SetUp(float3(0.0f, 1.0f, 0.0f));
	m_pCamera->SetRight(float3(1.0f, 0.0f, 0.0f));

	m_pMouse = m_pInputDriver->CreateMouseDevice(0);
	m_pKeybd = m_pInputDriver->CreateKeybdDevice(0);
	m_pMouse->Create((int_x)m_hWnd);
	m_pKeybd->Create((int_x)m_hWnd);

	object_ptr<I3DEntity> girl(new C3DEntity(m_pExplorer));
	girl->Load(L"girl.obj");
	girl->SetAction(0);
	//float4x4 girl_scl = float4x4_scale(float3(1.0f, 1.0f / 6.2f, 1.0f));
	float4x4 girl_pos = float4x4_translate(float3(1.0f, 13.5f, -17.0f));
	girl->Transform(girl_pos);

	object_ptr<I3DEntity> monster(new C3DEntity(m_pExplorer));
	monster->Load(L"monster.obj");
	monster->Transform(float4x4_translate(float3(14, 0, 0)));

	object_ptr<CETerrain> pTerrain(new CETerrain(m_pExplorer));
	pTerrain->Load(L"default.ter");

	m_pTerrain = pTerrain;
	SafeAddRef(m_pTerrain);

	object_ptr<CEVegetation>m_pVeg(new CEVegetation(m_pExplorer));
	m_pVeg->Load(L"flower.mesh", L"veg/flower.mtl", L"default/flower.veg");
	girl_pos[3][0] = -2;
	girl_pos[3][1] = 13.7f;
	girl_pos[3][2] = -15;
	m_pVeg->Transform(girl_pos);

	m_pScene = new C3DScene(this, m_pExplorer);
	m_pScene->SetCamera(m_pCamera);
	//m_pScene->AddNode(monster);
	m_pScene->AddNode(pTerrain);
	m_pScene->AddNode(girl);
	m_pScene->AddNode(m_pVeg);

	m_pMouse->OnMouseMove().connect(this, &C3DApp::OnMouseMove);
	m_pMouse->OnButtonToggle().connect(this, &C3DApp::OnButtonToggle);
	m_pKeybd->OnKeyToggle().connect(this, &C3DApp::OnKeyToggle);

	return err_ok;
}

err_t C3DApp::OnUninitialize()
{
	if(m_pScriptDriver)
		m_pScriptDriver->OnUninitialize();

	if(m_pMouse)
	{
		m_pMouse->OnButtonToggle().disconnect(this, &C3DApp::OnButtonToggle);
		m_pMouse->OnMouseMove().disconnect(this, &C3DApp::OnMouseMove);
	}

	if(m_pKeybd)
	{
		m_pKeybd->OnKeyToggle().disconnect(this, &C3DApp::OnKeyToggle);
	}

	SafeRelease(m_pTerrain);
	SafeRelease(m_pCamera);
	SafeRelease(m_pScene);

	SafeRelease(m_pMouse);
	SafeRelease(m_pKeybd);
	SafeRelease(m_pRenderTarget);
	SafeRelease(m_pFrameBuffer);

	SafeRelease(m_pExplorer);
	SafeRelease(m_pVedio);
	SafeRelease(m_pOutput);
	SafeRelease(m_pAdapter);

	SafeRelease(m_pScriptDriver);

	SafeRelease(m_pVedioDriver);
	SafeRelease(m_pInputDriver);
	SafeRelease(m_pFileDriver);
	SafeRelease(m_pApp);
	return err_ok;
}

err_t C3DApp::OnIdle(int_x iPeriod)
{
	err_t err = err_ok;
	if(m_pScriptDriver)
		err = m_pScriptDriver->OnIdle(iPeriod);
	if(err)
		return err;

	// win.10 WM_QUIT ²»»á·µ»Ø FALSE¡£
	MSG msg = {};
	while(::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) && msg.message != WM_QUIT)
	{
		switch(msg.message)
		{
		case MessageResize:
			if(msg.wParam != m_rect.w || msg.lParam != m_rect.h)
				ResizeFrameBuffer(msg.wParam, msg.lParam);
			break;
		default:
			Window::HandleMSG(msg);
			break;
		}
	}
	if(msg.message == WM_QUIT)
		return err_false;
	else
		return err_ok;
}

err_t C3DApp::OnUpdate(int_x iPeriod)
{
	if(m_pScriptDriver)
		m_pScriptDriver->OnUpdate(iPeriod);

	if(m_pMouse)
		m_pMouse->OnUpdate(iPeriod);
	if(m_pKeybd)
		m_pKeybd->OnUpdate(iPeriod);

	if(m_pKeybd)
	{
		float_32 fMoveSpeed = 1.0f;
		if(m_pKeybd->IsKeyDown(KeyCodeShift))
			fMoveSpeed *= 10;
		if(m_pKeybd->IsKeyDown(KeyCodeControl))
			fMoveSpeed *= 100;
		float_32 fRotSpeed = 3.1415926f * 0.5f;
		if(m_pKeybd->IsKeyDown(KeyCodeW))
			m_pCamera->ShiftZ(iPeriod * 0.001f * fMoveSpeed);
		if(m_pKeybd->IsKeyDown(KeyCodeA))
			m_pCamera->ShiftX(-iPeriod * 0.001f * fMoveSpeed);
		if(m_pKeybd->IsKeyDown(KeyCodeS))
			m_pCamera->ShiftZ(-iPeriod * 0.001f * fMoveSpeed);
		if(m_pKeybd->IsKeyDown(KeyCodeD))
			m_pCamera->ShiftX(iPeriod * 0.001f * fMoveSpeed);
		if(m_pKeybd->IsKeyDown(KeyCodeUp))
			m_pCamera->RollX(-iPeriod * 0.001f * fRotSpeed);
		if(m_pKeybd->IsKeyDown(KeyCodeDown))
			m_pCamera->RollX(iPeriod * 0.001f * fRotSpeed);
		if(m_pKeybd->IsKeyDown(KeyCodeLeft))
			m_pCamera->RollY(-iPeriod * 0.001f * fRotSpeed);
		if(m_pKeybd->IsKeyDown(KeyCodeRight))
			m_pCamera->RollY(iPeriod * 0.001f * fRotSpeed);
	}

	if(m_pScene)
		m_pScene->Update(iPeriod);
	return 0;
}

err_t C3DApp::OnRender(int_x iPeriod)
{
	if(m_pScriptDriver)
		m_pScriptDriver->OnRender(iPeriod);

	I3DContext * pContext = m_pVedio->GetContext();

	m_pRenderTarget->Bind(pContext);
	m_pRenderTarget->ClearRenderTarget(0, 0xFF008888, true, 1.0f, false, 0);
	//pContext->SetViewPort(0, 0, m_rect.w, m_rect.h);
	pContext->SetViewPort(100, 100, m_rect.w - 200, m_rect.h - 200);

	if(m_pScene)
		m_pScene->Render(pContext);

	m_pRenderTarget->Bind(nullptr);
	m_pFrameBuffer->Present();
	return 0;
}

int_x C3DApp::OnMouseMove(I3DInputMouse * pInput, int_x iOffX, int_x iOffY, int_x iOffZ)
{
	pointix point = pInput->GetMousePos();
	//Printf(L"x = %d, y = %d.\n", point.X, point.Y);
	if(pInput->IsButtonDown(MouseButtonR))
	{
		m_pCamera->RollX(iOffY * 0.005f);
		m_pCamera->RollY(iOffX * 0.005f);
	}
	m_pCamera->ShiftZ(iOffZ * 0.01f);
	return 0;
}

int_x C3DApp::OnButtonToggle(I3DInputMouse * pInput, MouseButtonE eButton, bool bPressed)
{
	if(!bPressed)
		return 0;

	if(eButton == MouseButtonM)
	{
		m_pCamera->SetPosition(float3(0.0f, 20.0f, -20.0f));
		m_pCamera->SetLook(float3(0.0f, -1.0f, 1.0f));
		m_pCamera->SetUp(float3(0.0f, 1.0f, 0.0f));
		m_pCamera->SetRight(float3(1.0f, 0.0f, 0.0f));
	}
	return 0;
}

int_x C3DApp::OnKeyToggle(I3DInputKeybd * pInput, KeyCodeE eKey, bool bPressed)
{
	if(!bPressed)
		return 0;

	if(eKey == KeyCodeEscape)
		PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
	else if(eKey == KeyCodeZ)
	{
		float3 pos = m_pCamera->GetPosition();
		log0(L"x = %.6f, y = %.6f, z = %.6f.", pos.x, pos.y, pos.z);
	}
	else if(eKey == KeyCodeF3)
	{
		if(m_pTerrain->GetFillMode() == FillModeSolid)
			m_pTerrain->SetFillMode(FillModeWireframe);
		else
			m_pTerrain->SetFillMode(FillModeSolid);
	}
	return 0;
}

void C3DApp::ResizeFrameBuffer(int_x iWidth, int_x iHeight)
{
	if(!m_pRenderTarget)
		return;

	m_rect.w = iWidth;
	m_rect.h = iHeight;

	m_pRenderTarget->RestRenderTarget();
	m_pFrameBuffer->Resize(m_rect.w, m_rect.h);
	object_ptr<I3DTexture> pBackBuffer(m_pFrameBuffer->GetBackBuffer(0));
	m_pRenderTarget->SetBackBuffer(0, pBackBuffer);

	object_ptr<I3DTexture> pDepthStencil(m_pVedio->CreateTexture());
	pDepthStencil->Create(m_rect.w, m_rect.h, cmode_d24s8, 1, BufferUsageDefault, BufferBindDepthStencil, BufferAccessNone);
	m_pRenderTarget->SetDepthStencil(pDepthStencil);
}

static unsigned __stdcall Run3DAppThreadFun(void * pParam)
{
	thread_set_name(-1, L"Run3DAppThreadFun");
	C3DApp * pApp = (C3DApp *)pParam;
	return (unsigned)pApp->Run();
}
