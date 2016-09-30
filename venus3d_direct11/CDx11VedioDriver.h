#pragma once

#include "Dx11Inc.h"

VENUS_BEG

class VDX11_API CDx11VedioDriver : public ObjectT<I3DVedioDriver>
{
public:
	CDx11VedioDriver();
	~CDx11VedioDriver();

	err_t Initialize();
	I3DAdapter * EnumAdapter(int_x iIndex);
	I3DVedio * CreateVedio(VedioModeE eMode, I3DAdapter * pAdapter);
protected:
	HMODULE m_hDxgi;
	IDXGIFactory * m_pFactory;
};

VENUS_END
