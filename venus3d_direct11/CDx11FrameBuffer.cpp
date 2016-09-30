#include "stdafx.h"
#include "CDx11FrameBuffer.h"
#include "CDx11Texture.h"
#include "CDxgiOutput.h"

VENUS_BEG

CDx11FrameBuffer::CDx11FrameBuffer(CDx11Vedio * pVedio) :
	m_pVedio(pVedio), m_pSwapChain(nullptr)
{
	SafeAddRef(m_pVedio);
}

CDx11FrameBuffer::~CDx11FrameBuffer()
{
	SafeRelease(m_pSwapChain);
	SafeRelease(m_pVedio);
}

I3DVedio * CDx11FrameBuffer::GetVedio() const
{
	return m_pVedio;
}

int_x CDx11FrameBuffer::Present()
{
	if(!m_pVedio || !m_pSwapChain)
		return 1;

	HRESULT hResult = m_pSwapChain->Present(0, 0);
	return err_ok;
}

err_t CDx11FrameBuffer::Create(int_x iWindowId, int_x iWidth, int_x iHeight, cmode_e eColorMode, int_x iCount,
	multisample_t multisample)
{
	if(!iWindowId ||
		iWidth < 0 || iWidth > 0xFFFF ||
		iHeight < 0 || iHeight > 0xFFFF ||
		iCount < 0 || iCount > 0xFF ||
		multisample.count < 1 || multisample.quality < 0)
		return err_invalidarg;

	if(!m_pVedio)
		return err_not_initialized;

	IDXGIFactory * pFactory = m_pVedio->Dx11GetFactory();
	ID3D11Device * pDevice = m_pVedio->Dx11GetDevice();

	Device2DTypeE eDevice2DType = Device2DTypeUnknown;
	I2DDevice * pRenderService = Get2DDevice();
	if(pRenderService)
		eDevice2DType = pRenderService->GetType();

	m_uiFlags = 0;
	if(eDevice2DType == Device2DTypeGdi ||
		eDevice2DType == Device2DTypeGdip)
		m_uiFlags |= DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;

	DXGI_SWAP_CHAIN_DESC scDesc = {};
	scDesc.BufferDesc.Width = (uint_32)iWidth;
	scDesc.BufferDesc.Height = (uint_32)iHeight;
	scDesc.BufferDesc.RefreshRate.Numerator = 60;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;
	scDesc.BufferDesc.Format = DxgiMapColorMode(eColorMode);
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scDesc.SampleDesc.Count = (uint_32)multisample.count;
	scDesc.SampleDesc.Quality = (uint_32)multisample.quality;

	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = (uint_32)iCount;
	scDesc.OutputWindow = (HWND)iWindowId;
	scDesc.Windowed = TRUE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.Flags = m_uiFlags;

	HRESULT hResult = pFactory->CreateSwapChain(pDevice, &scDesc, &m_pSwapChain);

	m_color_mode = eColorMode;
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iCount = iCount;
	return hResult;
}

sizeix CDx11FrameBuffer::GetSize() const
{
	if(!m_pSwapChain)
		return sizeix();
	DXGI_SWAP_CHAIN_DESC scDesc = {};
	m_pSwapChain->GetDesc(&scDesc);
	return sizeix((int_x)scDesc.BufferDesc.Width, (int_x)scDesc.BufferDesc.Height);
}

err_t CDx11FrameBuffer::Resize(int_x iWidth, int_x iHeight)
{
	if(!m_pSwapChain)
		return err_not_initialized;

	HRESULT hResult = m_pSwapChain->ResizeBuffers((uint_32)m_iCount, (uint_32)iWidth, (uint_32)iHeight,
		DxgiMapColorMode(m_color_mode), m_uiFlags);
	return err_ok;
}

err_t CDx11FrameBuffer::SetFullScreen(bool bFullScreen, I3DOutput * pOutput)
{
	if(!m_pSwapChain)
		return err_not_initialized;

	//DXGI_MODE_DESC mDesc = {};
	//mDesc.Width = m_iWidth;
	//mDesc.Height = m_iHeight;
	//mDesc.RefreshRate.Numerator = 60;
	//mDesc.RefreshRate.Denominator = 1;
	//mDesc.Format = DxgiGetColorFormat(m_color_mode);
	//mDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	//mDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//m_pSwapChain->ResizeTarget(&mDesc);
	CDxgiOutput * pDxOutput = (CDxgiOutput *)pOutput;
	HRESULT hResult = m_pSwapChain->SetFullscreenState(bFullScreen, pDxOutput ? pDxOutput->DxGetOutput() : nullptr);
	return err_ok;
}

bool CDx11FrameBuffer::IsFullScreen() const
{
	if(!m_pSwapChain)
		return false;

	BOOL bFullScreen = FALSE;
	IDXGIOutput * pOutput = nullptr;
	HRESULT hResult = m_pSwapChain->GetFullscreenState(&bFullScreen, &pOutput);
	SafeRelease(pOutput);
	return bFullScreen != FALSE;
}

I3DTexture * CDx11FrameBuffer::GetBackBuffer(int_x iIndex)
{
	if(!m_pSwapChain)
		return nullptr;

	ID3D11Texture2D * pBackBuffer = nullptr;
	HRESULT hResult = m_pSwapChain->GetBuffer((uint_32)iIndex, __uuidof(ID3D11Texture2D), (void **)&pBackBuffer);
	if(pBackBuffer)
	{
		I3DTexture * pRenderTarget = new CDx11Texture(m_pVedio, pBackBuffer);
		SafeRelease(pBackBuffer);
		return pRenderTarget;
	}
	else
		return nullptr;
}

VENUS_END
