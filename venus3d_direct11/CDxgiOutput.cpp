#include "stdafx.h"
#include "CDxgiOutput.h"

VENUS_BEG

struct struct_RECT : public RECT
{
public:
	struct_RECT(const RECT & rect) :RECT(rect){}
	operator rectix() const { return rectix(left, top, right - left, bottom - top); }
};

CDxgiOutput::CDxgiOutput(IDXGIOutput * pOutput):
m_pOutput(pOutput)
{
	SafeAddRef(m_pOutput);
	if(pOutput)
	{
		DXGI_OUTPUT_DESC oDesc;
		pOutput->GetDesc(&oDesc);
		m_rect = (struct_RECT)(oDesc.DesktopCoordinates);
	}
}

CDxgiOutput::~CDxgiOutput()
{
	SafeRelease(m_pOutput);
}

rectix CDxgiOutput::GetCoordinates() const
{
	return m_rect;
}

VENUS_END
