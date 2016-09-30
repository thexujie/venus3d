#include "stdafx.h"
#include "C3DApp.h"
#include <locale.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam);

#include "callstack.h"

int MyMain()
{
	//symblehlp_initialize();
	//callstack_t stack;
	//stack.generate(nullptr);

	CWin32App app;

	HINSTANCE hInstance = Window::GetCurrentInstance();
	const wchar_t * szClassName = L"{0x3a031342, 0x1adf, 0x44aa, {0x83, 0xa8, 0x25, 0xbb, 0xe8, 0xf8, 0xc1, 0x2d}}";

	// Register Class
	WNDCLASSEXW wcex = {sizeof(WNDCLASSEX)};
	wcex.style = 0;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = /*(HBRUSH)GetStockObject(NULL_BRUSH);*/NULL;
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm = 0;

	// if there is an icon, load it
	wcex.hIcon = (HICON)LoadImage(hInstance, __TEXT("ico.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

	RegisterClassExW(&wcex);

	HWND hWnd = CreateWindowW(szClassName, NULL, WS_OVERLAPPEDWINDOW,
							  60, 90, 1280, 720,
							  NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	MSG msg = {0};
	while(true)
	{
		BOOL bRet = ::GetMessage(&msg, NULL, 0, 0);
		if(bRet < 0)
			return bRet;

		if(!bRet)
			return (int)msg.wParam;

		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);
	}

	return 0;
}

#ifdef _WINMAIN_
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
#else
int _tmain(int argc, _TCHAR* argv[])
#endif
{
	int iRet = 0;
#ifdef _DEBUG
	_CrtMemState stateOld, stateNew, stateDiff;
	_CrtMemCheckpoint(&stateOld);
	iRet = MyMain();
	_CrtMemCheckpoint(&stateNew);
	if(_CrtMemDifference(&stateDiff, &stateOld, &stateNew))
		_CrtMemDumpAllObjectsSince(&stateDiff);
#else
	iRet = MyMain();
#endif 
	return iRet;
}

C3DApp * pApp = nullptr;
void OnCreate(HWND hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	pApp = new C3DApp(hWnd);
	pApp->Beg3D();
}

void OnDestroy(HWND hWnd)
{
	pApp->Post(C3DApp::MessageQuit, 0, 0);
	pApp->End3D();
	delete pApp;
}

void OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(hWnd, &ps);

	EndPaint(hWnd, &ps);
}

void OnResize(HWND hWnd)
{
	if(pApp)
	{
		RECT rc;
		::GetClientRect(hWnd, &rc);
		pApp->Post(C3DApp::MessageResize, rc.right, rc.bottom);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
	switch(uiMessage)
	{
	case WM_ERASEBKGND:
		return 0;
	case WM_CREATE:
		OnCreate(hWnd);
		break;
	case WM_SETFOCUS:
		if(pApp)
			pApp->Post(C3DApp::MessageFocus, true, 0);
		break;
	case WM_KILLFOCUS:
		if(pApp)
			pApp->Post(C3DApp::MessageFocus, false, 0);
		break;
	case WM_DESTROY:
		OnDestroy(hWnd);
		break;
	case WM_PAINT:
		OnPaint(hWnd);
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;

		//case WM_ENTERSIZEMOVE:
	case WM_SYSCOMMAND:
		DefWindowProc(hWnd, uiMessage, wParam, lParam);
		if(pApp && (wParam == SC_MAXIMIZE || wParam == SC_RESTORE))
			OnResize(hWnd);
		break;
	case WM_EXITSIZEMOVE:
		OnResize(hWnd);
		break;
	default:
		return DefWindowProc(hWnd, uiMessage, wParam, lParam);
	}
	return 0;
}