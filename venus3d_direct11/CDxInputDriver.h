#pragma once

VENUS_BEG

const uint_32 DINPUT_OBJ_MOUSE = 6;
const uint_32 DINPUT_KEY_NUM = 256;

class VDX11_API CDxInputDriver : public ObjectT<I3DInputDriver>
{
public:
	CDxInputDriver();
	~CDxInputDriver();

	err_t Initialize();
	I3DInputMouse * CreateMouseDevice(int_x iIndex);
	I3DInputKeybd * CreateKeybdDevice(int_x iIndex);

private:
	CModule m_dinput;
	IDirectInput8W * m_pDInput;
};

#pragma pack(push, 4)
struct DIMouseDataT
{
	int_32 m_iMouseX;
	int_32 m_iMouseY;
	int_32 m_iMouseZ;
	bool m_bMouseX;
	bool m_bMouseY;
	bool m_bMouseZ;
};
#pragma pack(pop)
class CDxInputMouse : public ObjectT<I3DInputMouse>
{
public:
	CDxInputMouse(IDirectInputDevice8W * pMouse);
	~CDxInputMouse();

	err_t Create(int_x iWindowId);
	err_t OnUpdate(int_x iPeriod);

	bool IsButtonDown(MouseButtonE eButton) const;
	pointix GetMousePos() const;
	event<I3DInputMouse *, int_x, int_x, int_x> & OnMouseMove();
	event<I3DInputMouse *, MouseButtonE, bool> & OnButtonToggle();
private:
	int_x m_iWindowId;
	IDirectInputDevice8W * m_pMouse;
	bool m_bAcquired;
	DIMouseDataT m_mouse;
	pointix m_mousePos;

public:
	event<I3DInputMouse *, int_x, int_x, int_x> OnMouseMoveEvent;
	event<I3DInputMouse *, MouseButtonE, bool> OnMouseToggleEvent;
};

class CDxInputKeybd : public ObjectT<I3DInputKeybd>
{
public:
	CDxInputKeybd(IDirectInputDevice8W * pKeybd);
	~CDxInputKeybd();

	err_t Create(int_x iWindowId);
	err_t OnUpdate(int_x iPeriod);

	bool IsKeyDown(KeyCodeE eKey) const;
	event<I3DInputKeybd *, KeyCodeE, bool> & OnKeyToggle();
private:
	int_x m_iWindowId;
	IDirectInputDevice8W * m_pKeybd;
	bool m_bAcquired;
	uint_8 m_keys[DINPUT_KEY_NUM];

public:
	event<I3DInputKeybd *, KeyCodeE, bool> OnKeyToggleEvent;
};

VENUS_END
