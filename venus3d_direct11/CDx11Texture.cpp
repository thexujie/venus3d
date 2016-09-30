#include "stdafx.h"
#include "CDx11Texture.h"

VENUS_BEG

bool dx11_rule_dds(image_convert_rule_t * rule);
bool dx11_rule_bmp(image_convert_rule_t * rule);
bool dx11_rule_tga(image_convert_rule_t * rule);
bool dx11_rule_png(image_convert_rule_t * rule);
bool dx11_rule_jpg(image_convert_rule_t * rule);

CDx11Texture::CDx11Texture(CDx11Vedio * pDevice) :
	m_pDevice(pDevice),
	m_pTexture(nullptr), m_pSRV(nullptr), m_pRTV(nullptr), m_pDSV(nullptr)
{
	SafeAddRef(m_pDevice);
}

CDx11Texture::CDx11Texture(CDx11Vedio * pDevice, ID3D11Texture2D * pTexture) :
	m_pDevice(pDevice),
	m_pTexture(pTexture), m_pSRV(nullptr), m_pRTV(nullptr), m_pDSV(nullptr)
{
	SafeAddRef(m_pDevice);
	SafeAddRef(m_pTexture);
}

CDx11Texture::~CDx11Texture()
{
	SafeRelease(m_pSRV);
	SafeRelease(m_pRTV);
	SafeRelease(m_pDSV);
	SafeRelease(m_pTexture);
	SafeRelease(m_pDevice);
}

I3DVedio * CDx11Texture::GetVedio() const
{
	return m_pDevice;
}

TextureDescT CDx11Texture::GetDesc() const
{
	TextureDescT desc;
	if(m_pTexture)
	{
		D3D11_TEXTURE2D_DESC _desc = {};
		m_pTexture->GetDesc(&_desc);
		desc.size.w = (int_x)_desc.Width;
		desc.size.h = (int_x)_desc.Height;
		desc.CMode = DxgiMapColorMode(_desc.Format);
		desc.MipLevel = (int_x)_desc.MipLevels;
		desc.SampleDesc.Count = (int_x)_desc.SampleDesc.Count;
		desc.SampleDesc.Quality = (int_x)_desc.SampleDesc.Quality;
		desc.Usage = Dx11::Mapping(_desc.Usage);
		desc.Bind = Dx11::Mapping((D3D11_BIND_FLAG)_desc.BindFlags);
		desc.Access = Dx11::Mapping((D3D11_CPU_ACCESS_FLAG)_desc.CPUAccessFlags);
	}
	return desc;
}

err_t CDx11Texture::Load(I3DFile * pFile)
{
	SafeRelease(m_pSRV);
	SafeRelease(m_pRTV);
	SafeRelease(m_pDSV);
	SafeRelease(m_pTexture);

	if(!pFile)
		return err_invalidarg;

	const byte_t * pBuffer = pFile->GetBuffer();
	int_x iLength = pFile->GetSize();

	if(!pBuffer || iLength < 4)
		return err_bad_data;

	ID3D11Device * pDevice = m_pDevice->Dx11GetDevice();
	if(!pDevice)
		return err_nullptr;

	// dds format
	if(is_dds_data(pBuffer, iLength))
	{
		//dds_create(pBuffer, iLength, &m_data, dx11_rule_dds);

		const dds_header_t * header = (const dds_header_t *)pBuffer;
		if(header->magic != DDS_MAGIC)
			return 1;
		if(header->size != sizeof(dds_header_t) - sizeof(DDS_MAGIC))
			return 1;

		if((header->flags & DDSF_VOLUME) || header->caps.cubemap_flags)
			return err_bad_format;

		if(header->mipmap_count < 0 || 15 < header->mipmap_count)
			return err_bad_format;

		int_x length = iLength - sizeof(dds_header_t);
		const byte_t * buffer = pBuffer + sizeof(dds_header_t);
		const byte_t * buffer_end = buffer + length;

		cmode_e color_mode = dds_get_cmode(header->pixel_format);
		if(color_mode == cmode_dx10)
		{
			const dds_header_10_t * header10 = (const dds_header_10_t *)buffer;
			buffer += sizeof(dds_header_10_t);
			color_mode = dds_get_cmode(header10->dxgi_format);
		}
		int_x width = header->width;
		int_x height = header->height;
		int_x mipmap_count = header->mipmap_count;
		if(mipmap_count < 1)
			mipmap_count = 1;
		else if(mipmap_count >= DX11_MAX_MIPMAP)
			mipmap_count = DX11_MAX_MIPMAP;
		else {}

		D3D11_SUBRESOURCE_DATA resdatas[DX11_MAX_MIPMAP] = {};
		for(uint_32 cnt = 0; cnt < mipmap_count; ++cnt)
		{
			if(buffer >= buffer_end)
				return err_invalidarg;

			int_x pitch = 0;
			int_x row = 0;
			int_x col = 0;
			dds_get_pitch(color_mode, width, height, &row, &col, &pitch);
			D3D11_SUBRESOURCE_DATA & data = resdatas[cnt];
			data.pSysMem = buffer;
			data.SysMemPitch = (uint_32)pitch;
			buffer += pitch * row;

			width >>= 1;
			height >>= 1;
			if(width == 0)
				width = 1;
			if(height == 0)
				height = 1;
		}

		if(buffer > buffer_end)
			return err_bad_data;

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = header->width;
		desc.Height = header->height;
		desc.MipLevels = (uint_32)(mipmap_count ? mipmap_count : 1);
		desc.ArraySize = 1;
		desc.Format = DxgiMapColorMode(color_mode);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		HRESULT hResult = pDevice->CreateTexture2D(&desc, resdatas, &m_pTexture);
		if(hResult)
			return err_false;
		else
			return err_ok;
	}
	else
	{
		image_data_t data;

		err_t err = err_bad_format;
		image_format_e format = image_get_format(pBuffer, iLength);
		switch(format)
		{
		case image_format_bmp:
			err = bmp_create(pBuffer, iLength, &data, dx11_rule_bmp);
			break;
		case image_format_dds:
			err = dds_create(pBuffer, iLength, &data, dx11_rule_dds);
			break;
		case image_format_jpg:
			err = jpg_create(pBuffer, iLength, &data, dx11_rule_jpg);
			break;
		case image_format_png:
			err = png_create(pBuffer, iLength, &data, dx11_rule_png);
			break;
		case image_format_tga:
			err = tga_create(pBuffer, iLength, &data, dx11_rule_tga);
			break;
		default:
			break;
		}

		if(err)
			return err;

		if(!data.buffer)
			return err_no_data;

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = (UINT)data.width;
		desc.Height = (UINT)data.height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DxgiMapColorMode(data.dst_mode);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA resdata = {};
		resdata.pSysMem = data.buffer;
		resdata.SysMemPitch = (uint_32)data.pitch;
		HRESULT hResult = pDevice->CreateTexture2D(&desc, &resdata, &m_pTexture);
		image_free(data.buffer);
		if(hResult)
		{
			return err_false;
		}
		else
		{
			return err_ok;
		}
	}
}

err_t CDx11Texture::Create(sizeix size, cmode_e cmode, int_x iMipLevel,
	bufferusage_e eUsage, bufferbind_e eBind, bufferaccess_e eAccess,
	multisample_t multisample, int_x iFlags)
{
	if(size.w <= 0 || size.h <= 0)
		return err_invalidarg;

	ID3D11Device * pDevice = m_pDevice->Dx11GetDevice();
	if(!pDevice)
		return err_nullptr;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = (UINT)size.w;
	desc.Height = (UINT)size.h;
	desc.MipLevels = (uint_32)iMipLevel;
	desc.ArraySize = 1;
	desc.Format = DxgiMapColorMode(cmode);
	desc.SampleDesc.Count = (uint_32)multisample.count;
	desc.SampleDesc.Quality = (uint_32)multisample.quality;
	desc.Usage = Dx11::Mapping(eUsage);
	desc.BindFlags = Dx11::Mapping(eBind);
	desc.CPUAccessFlags = Dx11::Mapping(eAccess);
	desc.MiscFlags = 0;

	if(iFlags & BufferFlagGdiCompatible)
		desc.MiscFlags |= D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

	if(desc.Format == DXGI_FORMAT_UNKNOWN)
	{
		log2(L"Unsupport color format : %s", cmode_text(cmode));
		return err_bad_format;
	}

	HRESULT hResult = pDevice->CreateTexture2D(&desc, nullptr, &m_pTexture);
	if(SUCCEEDED(hResult))
	{
		Dx11SetDebugName(m_pTexture, chbuffera<64>().format("CDx11Texture[%x]", this));
		return err_ok;
	}
	else
	{
		log2(L"Failed to create texture (%s)", cmode_text(cmode));
		return err_false;
	}
}

err_t CDx11Texture::Map(MappedBufferT & buffer, buffermap_e eMap)
{
	ID3D11Device * pDevice = m_pDevice->Dx11GetDevice();
	if(!pDevice)
		return err_nullptr;

	ID3D11DeviceContext * pContext = nullptr;
	pDevice->GetImmediateContext(&pContext);
	if(!pContext)
		return err_nullptr;

	D3D11_MAPPED_SUBRESOURCE res = {};
	HRESULT hResult = pContext->Map(m_pTexture, 0, Dx11::Mapping(eMap), 0, &res);
	SafeRelease(pContext);
	if(hResult)
		return err_failed;

	buffer.Data = (byte_t *)res.pData;
	buffer.Pitch = (int_x)res.RowPitch;
	buffer.Lengh = (int_x)res.DepthPitch;
	return err_ok;
}

err_t CDx11Texture::Unmap()
{
	ID3D11Device * pDevice = m_pDevice->Dx11GetDevice();
	if(!pDevice)
		return err_nullptr;

	ID3D11DeviceContext * pContext = nullptr;
	pDevice->GetImmediateContext(&pContext);
	if(!pContext)
		return err_nullptr;
	pContext->Unmap(m_pTexture, 0);
	SafeRelease(pContext);
	return err_ok;
}

err_t CDx11Texture::Copy(I3DTexture * pTarget, const rectix & rect) const
{
#ifdef _DEBUG
	TextureDescT desc_src = this->GetDesc();
	TextureDescT desc_dst = pTarget->GetDesc();
	if(desc_src.size.w != desc_dst.size.w ||
		desc_src.size.h != desc_dst.size.h ||
		cmode_bits(desc_src.CMode) != cmode_bits(desc_dst.CMode))
		throw exp_illegal_operation();
#endif

	ID3D11Device * pDevice = m_pDevice->Dx11GetDevice();
	if(!pDevice)
		return err_nullptr;

	ID3D11DeviceContext * pContext = nullptr;
	pDevice->GetImmediateContext(&pContext);
	if(!pContext)
		return err_nullptr;

	if(rect.w <= 0 || rect.h <= 0)
	{
		pContext->CopyResource(((CDx11Texture *)pTarget)->m_pTexture, m_pTexture);
		SafeRelease(pContext);
		return err_ok;
	}
	else
	{
		SafeRelease(pContext);
		//pContext->CopySubresourceRegion()
	}
	return err_failed;
}

void CDx11Texture::BeforeGetView(D3D11_BIND_FLAG eBind)
{
	ID3D11Device * pDevice = m_pDevice->Dx11GetDevice();
	D3D11_TEXTURE2D_DESC desc = {};
	m_pTexture->GetDesc(&desc);

	HRESULT hResult = S_OK;
	if(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE && eBind == D3D11_BIND_SHADER_RESOURCE)
	{
		if(m_pSRV)
			return;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.BufferEx.NumElements = 1;
		hResult = pDevice->CreateShaderResourceView(m_pTexture, &srvDesc, &m_pSRV);
	}
	else if(desc.BindFlags & D3D11_BIND_RENDER_TARGET && eBind == D3D11_BIND_RENDER_TARGET)
	{
		if(m_pRTV)
			return;

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = desc.Format;
		if(desc.SampleDesc.Count > 1)
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
		else
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		hResult = pDevice->CreateRenderTargetView(m_pTexture, &rtvDesc, &m_pRTV);
	}
	else if(desc.BindFlags & D3D11_BIND_DEPTH_STENCIL && eBind == D3D11_BIND_DEPTH_STENCIL)
	{
		if(m_pDSV)
			return;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = desc.Format;
		if(desc.SampleDesc.Count > 1)
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		else
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		hResult = pDevice->CreateDepthStencilView(m_pTexture, &dsvDesc, &m_pDSV);
	}
	else
	{
	}
}

I2DRTarget * CDx11Texture::Get2DRTarget()
{
	if(m_pTexture)
	{
		IDXGISurface1 * pSurface1 = nullptr;
		m_pTexture->QueryInterface(__uuidof(IDXGISurface1), (void **)&pSurface1);
		if(pSurface1)
		{
			CDxgiSurface * pBuffer2D = new CDxgiSurface(pSurface1);
			SafeRelease(pSurface1);
			return pBuffer2D;
		}
	}
	return nullptr;
}

VENUS_END
