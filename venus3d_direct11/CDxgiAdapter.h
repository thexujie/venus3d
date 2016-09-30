#pragma once

#include "DxgiInc.h"

VENUS_BEG

class CDxgiAdapter: public ObjectT<I3DAdapter>
{
public:
	CDxgiAdapter(IDXGIAdapter * pAdapter);
	~CDxgiAdapter();

	I3DOutput * EnumOutput(int_x iIndex);
public:
	IDXGIAdapter * DxGetAdapter() { return m_pAdapter; }
protected:
	IDXGIAdapter * m_pAdapter;
};

VENUS_END
