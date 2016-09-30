#pragma once

#include "CDx11Vedio.h"

VENUS_BEG

class CDx11Buffer : public ObjectT<I3DBuffer>
{
public:
	CDx11Buffer(CDx11Vedio * pDevice);
	~CDx11Buffer();

	I3DVedio * GetVedio() const;

	err_t Create(int_x iStride, int_x iCount,
		bufferusage_e eUsage = bufferusage_default,
		bufferbind_e eBind = bufferbind_resource,
		bufferaccess_e eAccess = bufferaccess_none,
		cmode_e mode = cmode_invalid,
		const void * pData = nullptr,
		int_x iDataSize = 0,
		int_x iDataSliceSize = 0);
	err_t DestroyBuffer();

	int_x GetStride() const;
	int_x GetSize() const;
	cmode_e GetMode() const;
	err_t SetBuffer(int_x iOffset, int_x iLength, const void * pData);
public:
	ID3D11Buffer * Dx11GetBuffer() { return m_pBuffer; }
	ID3D11ShaderResourceView * Dx11GetSRV();
protected:
	CDx11Vedio * m_pDevice;
	ID3D11Buffer * m_pBuffer;
	cmode_e m_mode;
	ID3D11ShaderResourceView * m_pSRV;
	int_x m_iStride, m_iSize;
};

VENUS_END
