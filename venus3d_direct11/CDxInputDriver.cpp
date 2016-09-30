#include "stdafx.h"
#include "CDxInputDriver.h"

VENUS_BEG

#define member_off(type, member)   ((unsigned long)(&((type *)nullptr)->member))

typedef HRESULT(WINAPI * DirectInput8CreateFunT)(HINSTANCE hinst, DWORD dwVersion, 
	const IID & riidltf, void ** ppvOut, IUnknown * punkOuter);
static const GUID DINPUT_IDirectInput8W = {0xBF798031, 0x483A, 0x4DA2, {0xAA, 0x99, 0x5D, 0x64, 0xED, 0x36, 0x97, 0x00}};

static const GUID DINPUT_SysKeyboard = {0x6F1D2B61, 0xD5A0, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
static const GUID DINPUT_SysMouse = {0x6F1D2B60, 0xD5A0, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};
static const GUID DINPUT_Key = {0x55728220, 0xD33C, 0x11CF, {0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
static const GUID DINPUT_XAxis = {0xA36D02E0, 0xC9F3, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};
static const GUID DINPUT_YAxis = {0xA36D02E1, 0xC9F3, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};
static const GUID DINPUT_ZAxis = {0xA36D02E2, 0xC9F3, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};
static const GUID DINPUT_Button = {0xA36D02F0, 0xC9F3, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};

static int_x MapKeyCodeToDIK(KeyCodeE eKey);
static KeyCodeE MapKeyCodeFromDIK(int_x iDik);

#pragma region CDxInputDriver
CDxInputDriver::CDxInputDriver():
m_pDInput(nullptr)
{

}

CDxInputDriver::~CDxInputDriver()
{

}

err_t CDxInputDriver::Initialize()
{
	if(m_pDInput)
		return err_ok;

	if(!m_dinput.Load(L"dinput8.dll"))
		return err_no_file;

	HINSTANCE hInstance = Win32::GetCurrentInstance();
	if(!hInstance)
		return err_no_context;
	DirectInput8CreateFunT pfnCreate = (DirectInput8CreateFunT)m_dinput.GetProc("DirectInput8Create");
	if(!pfnCreate)
		return err_no_data;

	HRESULT hResult = pfnCreate(hInstance, DIRECTINPUT_HEADER_VERSION,
								DINPUT_IDirectInput8W, (void **)&m_pDInput, nullptr);
	if(!m_pDInput)
	{
		m_dinput.Free();
		return err_no_data;
	}
	else
		return err_ok;
}

I3DInputMouse * CDxInputDriver::CreateMouseDevice(int_x iIndex)
{
	if(!m_pDInput)
		return nullptr;

	IDirectInputDevice8W * pMouse = nullptr;
	HRESULT hResult = m_pDInput->CreateDevice(DINPUT_SysMouse, &pMouse, nullptr);
	if(!pMouse)
		return nullptr;

	return new CDxInputMouse(pMouse);
}

I3DInputKeybd * CDxInputDriver::CreateKeybdDevice(int_x iIndex)
{
	if(!m_pDInput)
		return nullptr;

	IDirectInputDevice8W * pKeybd = nullptr;
	HRESULT hResult = m_pDInput->CreateDevice(DINPUT_SysKeyboard, &pKeybd, nullptr);
	if(!pKeybd)
		return nullptr;
	
	return new CDxInputKeybd(pKeybd);
}
#pragma endregion

#pragma region CDxInputMouse
CDxInputMouse::CDxInputMouse(IDirectInputDevice8W * pMouse) :
m_pMouse(pMouse), m_iWindowId(0), m_bAcquired(false)
{

}

CDxInputMouse::~CDxInputMouse()
{
	if(m_bAcquired)
		m_pMouse->Unacquire();
	SafeRelease(m_pMouse);
	buffclr(m_mouse);
	m_iWindowId = 0;
}

err_t CDxInputMouse::Create(int_x iWindowId)
{
	if(!m_pMouse)
		return err_not_initialized;

#pragma warning(disable: 4302 4311)
	HRESULT hResult = S_OK;
	DIOBJECTDATAFORMAT objMouse[DINPUT_OBJ_MOUSE] =
	{
		{&DINPUT_XAxis, member_off(DIMouseDataT, m_iMouseX), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0},
		{&DINPUT_YAxis, member_off(DIMouseDataT, m_iMouseY), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0},
		{&DINPUT_ZAxis, member_off(DIMouseDataT, m_iMouseZ), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0},
		{&DINPUT_Button, member_off(DIMouseDataT, m_bMouseX), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0},
		{&DINPUT_Button, member_off(DIMouseDataT, m_bMouseY), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0},
		{&DINPUT_Button, member_off(DIMouseDataT, m_bMouseZ), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0},
	};
#pragma warning(default: 4302 4311)

	DIDATAFORMAT fmtMouse = {sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT)};
	fmtMouse.dwFlags = DIDF_RELAXIS;
	fmtMouse.dwDataSize = sizeof(DIMouseDataT);
	fmtMouse.dwNumObjs = DINPUT_OBJ_MOUSE;
	fmtMouse.rgodf = objMouse;
	hResult = m_pMouse->SetDataFormat(&fmtMouse);
	if(!hResult)
		hResult = m_pMouse->SetCooperativeLevel((HWND)iWindowId, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	m_iWindowId = iWindowId;
	// 不使用缓冲模式
	//DIPROPDWORD diProp;
	//diProp.diph.dwSize = sizeof(DIPROPDWORD);
	//diProp.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	//diProp.diph.dwObj = 0;
	//diProp.diph.dwHow = DIPH_DEVICE;
	//diProp.dwData = sizeof(DIMouseDataT);
	//hResult = m_pMouse->SetProperty(DIPROP_BUFFERSIZE, &diProp.diph);
	return err_ok;
}

err_t CDxInputMouse::OnUpdate(int_x iPeriod)
{
	if(!m_pMouse)
		return err_not_initialized;

	HRESULT hResult = S_OK;
	if(!m_bAcquired)
	{
		if(GetForegroundWindow() != (HWND)m_iWindowId)
			return err_ok;

		hResult = m_pMouse->Acquire();
		if(hResult)
		{
			log0(L"failed: C3DDirectInput::Ready on keyboard 0x%0x.", hResult);
			return err_ok;
		}

		hResult = m_pMouse->GetDeviceState(sizeof(DIMouseDataT), (void *)&m_mouse);
		m_bAcquired = true;
	}

	DIMouseDataT mouse;
	hResult = m_pMouse->GetDeviceState(sizeof(DIMouseDataT), (void *)&mouse);
	if(!hResult)
	{
		if(mouse.m_iMouseX || mouse.m_iMouseY || mouse.m_iMouseZ)
		{
			OnMouseMoveEvent(this, mouse.m_iMouseX, mouse.m_iMouseY, mouse.m_iMouseZ);
			m_mousePos.x += mouse.m_iMouseX;
			m_mousePos.y += mouse.m_iMouseY;
		}

		if(mouse.m_bMouseX != m_mouse.m_bMouseX)
			OnMouseToggleEvent(this, MouseButtonL, mouse.m_bMouseX);
		if(mouse.m_bMouseY != m_mouse.m_bMouseY)
			OnMouseToggleEvent(this, MouseButtonR, mouse.m_bMouseX);
		if(mouse.m_bMouseZ != m_mouse.m_bMouseZ)
			OnMouseToggleEvent(this, MouseButtonM, mouse.m_bMouseZ);
		m_mouse = mouse;
	}
	else
	{
		m_pMouse->Unacquire();
		buffclr(m_mouse);
		m_bAcquired = false;
	}

	return err_ok;
}

bool CDxInputMouse::IsButtonDown(MouseButtonE eButton) const
{
	switch(eButton)
	{
	case MouseButtonL:
		return m_mouse.m_bMouseX;
	case MouseButtonR:
		return m_mouse.m_bMouseY;
	case MouseButtonM:
		return m_mouse.m_bMouseZ;
	default:
		return false;
	}
}

pointix CDxInputMouse::GetMousePos() const
{
	return m_mousePos;
}

event<I3DInputMouse *, int_x, int_x, int_x> & CDxInputMouse::OnMouseMove()
{
	return OnMouseMoveEvent;
}

event<I3DInputMouse *, MouseButtonE, bool> & CDxInputMouse::OnButtonToggle()
{
	return OnMouseToggleEvent;
}

#pragma endregion

#pragma region CDxInputKeybd
CDxInputKeybd::CDxInputKeybd(IDirectInputDevice8W * pKeybd):
m_pKeybd(pKeybd), m_iWindowId(0), m_bAcquired(false)
{
	buffclr(m_keys);
}

CDxInputKeybd::~CDxInputKeybd()
{
	if(m_bAcquired)
		m_pKeybd->Unacquire();
	SafeRelease(m_pKeybd);
	buffclr(m_keys);
	m_iWindowId = 0;
}

err_t CDxInputKeybd::Create(int_x iWindowId)
{
	if(!m_pKeybd)
		return err_not_initialized;

	HRESULT hResult = S_OK;
	DIOBJECTDATAFORMAT objKeyboard[DINPUT_KEY_NUM] = {};
	for(uint_32 cnt = 0; cnt < DINPUT_KEY_NUM; ++cnt)
	{
		DWORD dwKey = (DWORD)MapKeyCodeToDIK((KeyCodeE)cnt);
		DIOBJECTDATAFORMAT & obj = objKeyboard[cnt];
		obj.pguid = &DINPUT_Key;
		obj.dwOfs = cnt;
		obj.dwType = 0x80000000 | DIDFT_BUTTON | DIDFT_MAKEINSTANCE(dwKey);
		obj.dwFlags = 0;
	}

	DIDATAFORMAT fmtKeyboard = {sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT)};
	fmtKeyboard.dwFlags = DIDF_RELAXIS;
	fmtKeyboard.dwDataSize = sizeof(m_keys);
	fmtKeyboard.dwNumObjs = DINPUT_KEY_NUM;
	fmtKeyboard.rgodf = objKeyboard;
	hResult = m_pKeybd->SetDataFormat(&fmtKeyboard);
	if(!hResult)
		hResult = m_pKeybd->SetCooperativeLevel((HWND)iWindowId, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	m_iWindowId = iWindowId;
	return err_ok;
}

err_t CDxInputKeybd::OnUpdate(int_x iPeriod)
{
	if(!m_pKeybd)
		return err_not_initialized;

	HRESULT hResult = S_OK;
	if(!m_bAcquired)
	{
		if(GetForegroundWindow() != (HWND)m_iWindowId)
			return err_ok;

		hResult = m_pKeybd->Acquire();
		if(hResult)
		{
			log0(L"failed: C3DDirectInput::Ready on keyboard 0x%0x.", hResult);
			return err_ok;
		}

		hResult = m_pKeybd->GetDeviceState(sizeof(m_keys), (void *)m_keys);
		m_bAcquired = true;
	}

	BYTE keys[DINPUT_KEY_NUM];
	hResult = m_pKeybd->GetDeviceState(DINPUT_KEY_NUM, (void *)keys);
	if(!hResult)
	{
		for(uint_32 cnt = 0; cnt != DINPUT_KEY_NUM; ++cnt)
		{
			if(keys[cnt] != m_keys[cnt])
			{
				OnKeyToggleEvent(this, KeyCodeE(cnt), (keys[cnt] & 0x80) != 0);
				m_keys[cnt] = keys[cnt];
			}
		}
	}
	else
	{
		m_pKeybd->Unacquire();
		buffclr(m_keys);
		m_bAcquired = false;
	}
	return err_ok;
}

bool CDxInputKeybd::IsKeyDown(KeyCodeE eKey) const
{
	if(eKey == KeyCodeShift)
		return ((m_keys[KeyCodeShiftL] & 0x80) == 0x80) || ((m_keys[KeyCodeShiftR] & 0x80) == 0x80);
	return (m_keys[eKey] & 0x80) == 0x80;
}

event<I3DInputKeybd *, KeyCodeE, bool> & CDxInputKeybd::OnKeyToggle()
{
	return OnKeyToggleEvent;
}


static int_x MapKeyCodeToDIK(KeyCodeE eKey)
{
	if(eKey < 0 || eKey >= KeyCodeCount)
		return KeyCodeInvalid;

	const int_x NOTMATCH = KeyCodeInvalid;
	const int_x RESERVED = KeyCodeInvalid;
	const int_x UNASSIGNED = KeyCodeInvalid;
	const int_x NONE = KeyCodeInvalid;

	int_x KEYS[0xFF] =
	{
		0,
		DIK_0,
		DIK_1,
		DIK_2,
		DIK_3,
		DIK_4,
		DIK_5,
		DIK_6,
		DIK_7,
		DIK_8,
		DIK_9,

		DIK_CAPITAL,

		DIK_A,
		DIK_B,
		DIK_C,
		DIK_D,
		DIK_E,
		DIK_F,
		DIK_G,
		DIK_H,
		DIK_I,
		DIK_J,
		DIK_K,
		DIK_L,
		DIK_M,
		DIK_N,
		DIK_O,
		DIK_P,
		DIK_Q,
		DIK_R,
		DIK_S,
		DIK_T,
		DIK_U,
		DIK_V,
		DIK_W,
		DIK_X,
		DIK_Y,
		DIK_Z,

		DIK_BACKSPACE,
		DIK_TAB,
		DIK_RETURN,
		DIK_ESCAPE,
		DIK_SPACE,

		DIK_CAPSLOCK,
		DIK_NUMPAD0,
		DIK_NUMPAD1,
		DIK_NUMPAD2,
		DIK_NUMPAD3,
		DIK_NUMPAD4,
		DIK_NUMPAD5,
		DIK_NUMPAD6,
		DIK_NUMPAD7,
		DIK_NUMPAD8,
		DIK_NUMPAD9,
		DIK_ADD,
		DIK_SUBTRACT,
		DIK_MULTIPLY,
		DIK_DIVIDE,
		DIK_DECIMAL,
		DIK_NUMPADENTER,
		DIK_NUMPADEQUALS,

		DIK_F1,
		DIK_F2,
		DIK_F3,
		DIK_F4,
		DIK_F5,
		DIK_F6,
		DIK_F7,
		DIK_F8,
		DIK_F9,
		DIK_F10,
		DIK_F11,
		DIK_F12,
		DIK_F13,
		DIK_F14,
		DIK_F15,
		NONE,
		NONE,
		NONE,
		NONE,
		NONE,
		NONE,
		NONE,
		NONE,
		NONE,

		DIK_MINUS,
		DIK_EQUALS,
		DIK_GRAVE,
		DIK_COMMA,
		DIK_PERIOD,
		DIK_SLASH,
		DIK_SEMICOLON,
		DIK_COLON,
		DIK_APOSTROPHE,
		DIK_LBRACKET,
		DIK_RBRACKET,
		DIK_BACKSLASH,

		DIK_INSERT,
		DIK_DELETE,
		DIK_HOME,
		DIK_END,
		DIK_PGUP,
		DIK_PGDN,
		DIK_LEFT,
		DIK_UP,
		DIK_RIGHT,
		DIK_DOWN,

		NONE,
		DIK_SCROLL,
		DIK_PAUSE,

		DIK_LWIN,
		DIK_RWIN,
		NONE,
		DIK_LCONTROL,
		DIK_RCONTROL,
		NONE,
		DIK_LSHIFT,
		DIK_RSHIFT,
		NONE,
		DIK_LALT,
		DIK_RALT,
	};
	return KEYS[eKey];
}

static KeyCodeE MapKeyCodeFromDIK(int_x iDik)
{
	if(iDik < 0 || iDik >= 0xFF)
		return KeyCodeInvalid;

	const KeyCodeE NOTMATCH = KeyCodeInvalid;
	const KeyCodeE RESERVED = KeyCodeInvalid;
	const KeyCodeE UNASSIGNED = KeyCodeInvalid;
	const KeyCodeE NONE = KeyCodeInvalid;

	KeyCodeE KEYS[0xFF] =
	{
		/*0x00*/NONE,
		/*0x01*/KeyCodeEscape,
		/*0x02*/KeyCode1,
		/*0x03*/KeyCode2,
		/*0x04*/KeyCode3,
		/*0x05*/KeyCode4,
		/*0x06*/KeyCode5,
		/*0x07*/KeyCode6,
		/*0x08*/KeyCode7,
		/*0x09*/KeyCode8,
		/*0x0A*/KeyCode9,
		/*0x0B*/KeyCode0,
		/*0x0C*/KeyCodeSub,
		/*0x0D*/KeyCodeEqual,
		/*0x0E*/KeyCodeBackspace,
		/*0x0F*/KeyCodeTab,

		/*0x10*/KeyCodeQ,
		/*0x11*/KeyCodeW,
		/*0x12*/KeyCodeEe,
		/*0x13*/KeyCodeR,
		/*0x14*/KeyCodeT,
		/*0x15*/KeyCodeY,
		/*0x16*/KeyCodeU,
		/*0x17*/KeyCodeI,
		/*0x18*/KeyCodeO,
		/*0x19*/KeyCodeP,
		/*0x1A*/KeyCodeBracketL,
		/*0x1B*/KeyCodeBracketR,
		/*0x1C*/KeyCodeEnter,
		/*0x1D*/KeyCodeControlL,
		/*0x1E*/KeyCodeA,
		/*0x1F*/KeyCodeS,

		/*0x20*/KeyCodeD,
		/*0x21*/KeyCodeF,
		/*0x22*/KeyCodeG,
		/*0x23*/KeyCodeH,
		/*0x24*/KeyCodeJ,
		/*0x25*/KeyCodeK,
		/*0x26*/KeyCodeL,
		/*0x27*/KeyCodeSemicolon,
		/*0x28*/KeyCodeApostrophe,
		/*0x29*/NONE,
		/*0x2A*/KeyCodeShiftL,
		/*0x2B*/KeyCodeBackslash,
		/*0x2C*/KeyCodeZ,
		/*0x2D*/KeyCodeX,
		/*0x2E*/KeyCodeC,
		/*0x2F*/KeyCodeV,

		/*0x30*/KeyCodeB,
		/*0x31*/KeyCodeN,
		/*0x32*/KeyCodeM,
		/*0x33*/KeyCodeComma,
		/*0x34*/KeyCodePeriod,
		/*0x35*/KeyCodeSlash,
		/*0x36*/KeyCodeShiftR,
		/*0x37*/KeyCodeNumMul,
		/*0x38*/KeyCodeAltL,
		/*0x39*/KeyCodeSpace,
		/*0x3A*/KeyCodeCapsLock,
		/*0x3B*/KeyCodeF1,
		/*0x3C*/KeyCodeF2,
		/*0x3D*/KeyCodeF3,
		/*0x3E*/KeyCodeF4,
		/*0x3F*/KeyCodeF5,

		/*0x40*/KeyCodeF6,
		/*0x41*/KeyCodeF7,
		/*0x42*/KeyCodeF8,
		/*0x43*/KeyCodeF9,
		/*0x44*/KeyCodeF10,
		/*0x45*/KeyCodeNumlock,
		/*0x46*/KeyCodeScrollLock,
		/*0x47*/KeyCodeNum7,
		/*0x48*/KeyCodeNum8,
		/*0x49*/KeyCodeNum9,
		/*0x4A*/KeyCodeNumSub,
		/*0x4B*/KeyCodeNum4,
		/*0x4C*/KeyCodeNum5,
		/*0x4D*/KeyCodeNum6,
		/*0x4E*/KeyCodeNumAdd,
		/*0x4F*/KeyCodeNum1,

		/*0x50*/KeyCodeNum2,
		/*0x51*/KeyCodeNum3,
		/*0x52*/KeyCodeNum0,
		/*0x53*/KeyCodeNumDecimal,
		/*0x54*/NONE,
		/*0x55*/NONE,
		/*0x56*/NONE,
		/*0x57*/KeyCodeF11,
		/*0x58*/KeyCodeF12,
		/*0x59*/KeyCodeF13,
		/*0x5A*/KeyCodeF14,
		/*0x5B*/KeyCodeF15,
		/*0x5C*/NONE,
		/*0x5D*/NONE,
		/*0x5E*/
		/*0x5F*/
	};

	return KEYS[iDik];
}
#pragma endregion

VENUS_END