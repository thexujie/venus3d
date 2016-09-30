#pragma once

#include "Dx11Inc.h"
#include "CDx11Vedio.h"

VENUS_BEG

class CDx11FrameBuffer : public ObjectT<I3DFrameBuffer>
{
public:
	CDx11FrameBuffer(CDx11Vedio * pVedio);
	~CDx11FrameBuffer();
	I3DVedio * GetVedio() const;

	int_x Present();
	err_t Create(int_x iWindowId, int_x iWidth, int_x iHeight, cmode_e eColorMode, int_x iCount,
		multisample_t multisample = multisample_1);
	sizeix GetSize() const;
	err_t Resize(int_x iWidth, int_x iHeight);
	err_t SetFullScreen(bool bFullScreen, I3DOutput * pOutput);
	bool IsFullScreen() const;

	I3DTexture * GetBackBuffer(int_x iIndex);
protected:
	CDx11Vedio * m_pVedio;
	IDXGISwapChain * m_pSwapChain;
	cmode_e m_color_mode;
	uint_32 m_uiFlags;

	int_x m_iWindowId, m_iWidth, m_iHeight, m_iCount;
};

VENUS_END
