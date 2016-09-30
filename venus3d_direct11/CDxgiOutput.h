#pragma once

#include "DxgiInc.h"

VENUS_BEG

class CDxgiOutput : public ObjectT<I3DOutput>
{
public:
	CDxgiOutput(IDXGIOutput * pOutput);
	~CDxgiOutput();

	rectix GetCoordinates() const;
public:
	IDXGIOutput * DxGetOutput() { return m_pOutput; }
protected:
	IDXGIOutput * m_pOutput;
	rectix m_rect;
};

VENUS_END
