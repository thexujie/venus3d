#pragma once

#include "CDx11Vedio.h"

VENUS_BEG

class CDx11Texture : public ObjectT<I3DTexture>
{
public:
	CDx11Texture(CDx11Vedio * pDevice);
	CDx11Texture(CDx11Vedio * pDevice, ID3D11Texture2D * pTexture);
	~CDx11Texture();

	I3DVedio * GetVedio() const;

	TextureDescT GetDesc() const;
	err_t Load(I3DFile * pFile);
	err_t Create(sizeix size, cmode_e cmode, int_x iMipLevel = 1,
		bufferusage_e eUsage = bufferusage_default,
		bufferbind_e eBind = bufferbind_resource,
		bufferaccess_e eAccess = bufferaccess_none, 
		multisample_t multisample = multisample_1,
		int_x iFlags = 0);
	err_t Map(MappedBufferT & buffer, buffermap_e eMap);
	err_t Unmap();
	err_t Copy(I3DTexture * pTarget, const rectix & rect = rectix()) const;

	I2DRTarget * Get2DRTarget();

public:
	void BeforeGetView(D3D11_BIND_FLAG eBind);
	ID3D11ShaderResourceView * Dx11GetSRV() { BeforeGetView(D3D11_BIND_SHADER_RESOURCE); return m_pSRV; }
	ID3D11RenderTargetView * Dx11GetRTV() { BeforeGetView(D3D11_BIND_RENDER_TARGET); return m_pRTV; }
	ID3D11DepthStencilView * Dx11GetDSV() { BeforeGetView(D3D11_BIND_DEPTH_STENCIL); return m_pDSV; }
private:
	CDx11Vedio * m_pDevice;

	ID3D11Texture2D * m_pTexture;
	ID3D11ShaderResourceView * m_pSRV;
	ID3D11RenderTargetView * m_pRTV;
	ID3D11DepthStencilView * m_pDSV;
};

VENUS_END
