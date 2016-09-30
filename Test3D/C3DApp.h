#pragma once

#include "BaseInc.h"
#include "CETerrain.h"
#include "CEVegetation.h"

class C3DApp : public ObjectT<IApp3D>
{
public:
	C3DApp(HWND hWnd);
	~C3DApp();

	I3DFileDriver * GetFileDriver() const;
	I3DVedioDriver * GetVedioDriver() const;
	I3DAudioDriver * GetAudioDriver() const;
	I3DInputDriver * GetInputDriver() const;
	I3DExplorer * CreateExplorer(I3DVedio * pVedio);
	I3DScene * CreateScene(I3DExplorer * pExplorer);

	err_t Beg3D();
	err_t End3D();

	err_t Post(uint_32 uiMessage, uint_x uiParam, int_x iParam);

	err_t Run();
	err_t OnInitialize();
	err_t OnUninitialize();
	err_t OnIdle(int_x iPeriod);
	err_t OnUpdate(int_x iPeriod);
	err_t OnRender(int_x iPeriod);
public:
	int_x OnMouseMove(I3DInputMouse * pInput, int_x iOffX, int_x iOffY, int_x iOffZ);
	int_x OnButtonToggle(I3DInputMouse * pInput, MouseButtonE eButton, bool bPressed);
	int_x OnKeyToggle(I3DInputKeybd * pInput, KeyCodeE eKey, bool bPressed);

private:
	void ResizeFrameBuffer(int_x iWidth, int_x iHeight);

public:
	enum MessageE
	{
		MessageNull = WM_USER,
		MessageQuit,
		MessageResize,
		MessageFocus, // (bool bFocus)
	};
private:
	IApp * m_pApp;

	int_x m_iThread;
	uint_32 m_uiThreadId;

	HWND m_hWnd;
	rectix m_rect;

	I3DFileDriver * m_pFileDriver;
	I3DVedioDriver * m_pVedioDriver;
	I3DInputDriver * m_pInputDriver;
	IPlugin * m_pScriptDriver;

	I3DExplorer * m_pExplorer;

	I3DVedio * m_pVedio;
	I3DAdapter * m_pAdapter;
	I3DOutput * m_pOutput;

	I3DFrameBuffer * m_pFrameBuffer;
	I3DRenderTarget * m_pRenderTarget;

	I3DInputMouse * m_pMouse;
	I3DInputKeybd * m_pKeybd;

	I3DScene * m_pScene;
	I3DCamera * m_pCamera;
	I3DNode * m_pTerrain;
};
