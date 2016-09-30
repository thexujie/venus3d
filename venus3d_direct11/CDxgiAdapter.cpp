#include "stdafx.h"
#include "CDxgiAdapter.h"
#include "CDxgiOutput.h"

VENUS_BEG

CDxgiAdapter::CDxgiAdapter(IDXGIAdapter * pAdapter) :
m_pAdapter(pAdapter)
{
	SafeAddRef(m_pAdapter);
	if(pAdapter)
	{
		DXGI_ADAPTER_DESC aDesc = {};
		pAdapter->GetDesc(&aDesc);
		log0(L"%s.", aDesc.Description);
	}
}

CDxgiAdapter::~CDxgiAdapter()
{
	SafeRelease(m_pAdapter);
}

I3DOutput * CDxgiAdapter::EnumOutput(int_x iIndex)
{
	if(!m_pAdapter)
		return nullptr;

	IDXGIOutput * pOutput = nullptr;
	HRESULT hResult = m_pAdapter->EnumOutputs((uint_32)iIndex, &pOutput);
	if(hResult == DXGI_ERROR_NOT_FOUND)
		return nullptr;
	else
		return new CDxgiOutput(pOutput);
}

VENUS_END
