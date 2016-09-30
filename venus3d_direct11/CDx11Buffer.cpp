#include "stdafx.h"
#include "CDx11Buffer.h"
#include "CDx11Context.h"

VENUS_BEG

CDx11Buffer::CDx11Buffer(CDx11Vedio * pDevice) :
	m_pDevice(pDevice),
	m_iStride(0), m_iSize(0),
	m_pBuffer(nullptr), m_pSRV(nullptr), m_mode(cmode_invalid)
{
	SafeAddRef(m_pDevice);
}

CDx11Buffer::~CDx11Buffer()
{
	SafeRelease(m_pSRV);
	SafeRelease(m_pBuffer);
	SafeRelease(m_pDevice);
}

I3DVedio * CDx11Buffer::GetVedio() const
{
	return m_pDevice;
}

err_t CDx11Buffer::Create(int_x iStride, int_x iCount,
	bufferusage_e eUsage,
	bufferbind_e eBind,
	bufferaccess_e eAccess,
	cmode_e mode,
	const void * pData,
	int_x iDataSize,
	int_x iDataSliceSize)
{
	ID3D11Device * pDevice = m_pDevice->Dx11GetDevice();
	Assert(pDevice);
	if(!pDevice)
		return -1;

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = (UINT)(iStride * iCount);
	desc.Usage = Dx11::Mapping(eUsage);
	desc.BindFlags = Dx11::Mapping(eBind);
	desc.CPUAccessFlags = Dx11::Mapping(eAccess);
	desc.MiscFlags = 0;
	desc.StructureByteStride = (UINT)iStride;

	HRESULT hResult = S_OK;
	if(pData)
	{
		D3D11_SUBRESOURCE_DATA srd = {};
		srd.pSysMem = pData;
		srd.SysMemPitch = (UINT)iDataSize;
		srd.SysMemSlicePitch = (UINT)iDataSliceSize;
		hResult = pDevice->CreateBuffer(&desc, &srd, &m_pBuffer);
	}
	else
		hResult = pDevice->CreateBuffer(&desc, nullptr, &m_pBuffer);
	if(FAILED(hResult))
		return err_failed;
	else
	{
		m_mode = mode;
		m_iStride = iStride;
		m_iSize = iStride * iCount;
		Dx11SetDebugName(m_pBuffer, chbuffera<64>().format("CDx11Buffer[%x]", this));
		return err_ok;
	}
}

err_t CDx11Buffer::DestroyBuffer()
{
	SafeRelease(m_pBuffer);
	m_iStride = 0;
	m_iSize = 0;
	return err_ok;
}

int_x CDx11Buffer::GetStride() const
{
	return m_iStride;
}

int_x CDx11Buffer::GetSize() const
{
	return m_iSize;
}

cmode_e CDx11Buffer::GetMode() const
{
	return m_mode;
}

err_t CDx11Buffer::SetBuffer(int_x iOffset, int_x iLength, const void * pData)
{
	if(iOffset < 0 || iLength < 0 || !pData)
		return err_invalidarg;

	if(!m_pBuffer)
		return err_no_data;

	ID3D11DeviceContext * pContext = nullptr;
	m_pDevice->Dx11GetDevice()->GetImmediateContext(&pContext);
	if(!pContext)
		return err_nullptr;

	D3D11_MAPPED_SUBRESOURCE res = {};
	HRESULT hResult = pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	if(hResult == S_OK && iOffset + iLength <= res.RowPitch)
	{
		buffcpy8(res.pData, iLength, pData, iLength);
		pContext->Unmap(m_pBuffer, 0);
	}
	SafeRelease(pContext);
	return err_ok;
}

ID3D11ShaderResourceView * CDx11Buffer::Dx11GetSRV()
{
	ID3D11Device * pDevice = m_pDevice->Dx11GetDevice();
	if(m_pBuffer && pDevice)
	{
		if(!m_pSRV)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.ViewDimension = D3D_SRV_DIMENSION_BUFFER;
			srvDesc.Format = DxgiMapColorMode(m_mode);
			srvDesc.Buffer.ElementOffset = 0;
			srvDesc.Buffer.NumElements = (uint_32)(m_iSize * 8 / cmode_bits(m_mode));
			winerr_t err = pDevice->CreateShaderResourceView(m_pBuffer, &srvDesc, &m_pSRV);
			if(err.failed())
				log2(L"CDx11Buffer::Dx11GetSRV failed!");
		}
	}
	else {}
	return m_pSRV;
}

VENUS_END
