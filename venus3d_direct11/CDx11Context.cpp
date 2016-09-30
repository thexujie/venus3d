#include "stdafx.h"
#include "CDx11Context.h"
#include "CDx11Buffer.h"
#include "CDx11Texture.h"
#include "CDx11Effect.h"
#include "CDx11Pass.h"
VENUS_BEG

CDx11Context::CDx11Context(CDx11Vedio * pVedio, ID3D11DeviceContext * pContext) :
	m_pVedio(pVedio), m_pContext(pContext), m_pDepthStencil(nullptr), m_pCamera(nullptr),
	m_pPass(nullptr)
{
	SafeAddRef(m_pVedio);
	SafeAddRef(m_pContext);
	arrayfill<I3DTexture *>(m_pBackBuffers, nullptr);
}

CDx11Context::~CDx11Context()
{
	Destroy();
	for(I3DTexture *& pBackBuffer : m_pBackBuffers) SafeRelease(pBackBuffer);
	SafeRelease(m_pDepthStencil);
	SafeRelease(m_pContext);
	SafeRelease(m_pVedio);
	SafeRelease(m_pCamera);
}

I3DVedio * CDx11Context::GetVedio() const
{
	return m_pVedio;
}

void CDx11Context::OnDeviceLost()
{
	Destroy();
}

void CDx11Context::OnDeviceReset()
{
	Create();
}

void CDx11Context::Create()
{
}

void CDx11Context::Destroy()
{
	ClearStates();
}

err_t CDx11Context::SetViewPort(rectix viewport)
{
	if(!m_pContext)
		return err_not_initialized;

	D3D11_VIEWPORT d3dvp = {};
	d3dvp.TopLeftX = (float_32)viewport.x;
	d3dvp.TopLeftY = (float_32)viewport.y;
	d3dvp.Width = (float_32)viewport.w;
	d3dvp.Height = (float_32)viewport.h;
	d3dvp.MinDepth = 0.0f;
	d3dvp.MaxDepth = 1.0f;
	m_pContext->RSSetViewports(1, &d3dvp);
	return err_ok;
}

err_t CDx11Context::SetRenderTarget(I3DTexture ** ppBackBuffer, int_x iCount, I3DTexture * pDepthStencil)
{
	if(!m_pContext)
		return err_not_initialized;

	if(iCount < 0 || iCount > DX11_RENDER_TARGET_COUNT)
		return err_invalidarg;

	for(I3DTexture *& pBackBuffer : m_pBackBuffers) SafeRelease(pBackBuffer);
	arraycpy(m_pBackBuffers, DX11_RENDER_TARGET_COUNT, ppBackBuffer, iCount);
	for(I3DTexture *& pBackBuffer : m_pBackBuffers) SafeAddRef(pBackBuffer);

	SafeRelease(m_pDepthStencil);
	m_pDepthStencil = pDepthStencil;
	SafeAddRef(m_pDepthStencil);

	ID3D11RenderTargetView * rtvs[DX11_RENDER_TARGET_COUNT] = {};
	ID3D11DepthStencilView * pdsv = nullptr;

	for(int_x cnt = 0; cnt < DX11_RENDER_TARGET_COUNT; ++cnt)
	{
		CDx11Texture * prt = (CDx11Texture *)m_pBackBuffers[cnt];
		if(prt)
			rtvs[cnt] = prt->Dx11GetRTV();
	}
	CDx11Texture * pds = (CDx11Texture *)m_pDepthStencil;
	if(pds)
		pdsv = pds->Dx11GetDSV();
	m_pContext->OMSetRenderTargets(DX11_RENDER_TARGET_COUNT, rtvs, pdsv);
	return err_ok;
}

err_t CDx11Context::SetBackBuffer(I3DTexture * pBackBuffer, int_x iIndex)
{
	if(!m_pContext)
		return err_not_initialized;

	SafeRelease(m_pBackBuffers[iIndex]);
	m_pBackBuffers[iIndex] = pBackBuffer;
	SafeAddRef(m_pBackBuffers[iIndex]);

	if(!pBackBuffer)
		return err_ok;

	CDx11Texture * pdxTexture = (CDx11Texture *)pBackBuffer;

	ID3D11RenderTargetView * prtv = pdxTexture->Dx11GetRTV();
	m_pContext->OMSetRenderTargets(1, &prtv, nullptr);
	return err_ok;
}

err_t CDx11Context::SetDepthStencil(I3DTexture * pDepthStencil)
{
	if(!m_pContext)
		return err_not_initialized;

	SafeRelease(m_pDepthStencil);
	m_pDepthStencil = pDepthStencil;
	SafeAddRef(m_pDepthStencil);

	if(!pDepthStencil)
		return err_ok;

	CDx11Texture * pdxTexture = (CDx11Texture *)pDepthStencil;
	ID3D11DepthStencilView * pdsv = pdxTexture->Dx11GetDSV();
	m_pContext->OMSetRenderTargets(0, 0, pdsv);
	return err_ok;
}

err_t CDx11Context::SetCamera(I3DCamera * pCamera)
{
	SafeRelease(m_pCamera);
	m_pCamera = pCamera;
	SafeAddRef(m_pCamera);
	return err_ok;
}

I3DCamera * CDx11Context::GetCamera() const
{
	SafeAddRef(m_pCamera);
	return m_pCamera;
}

err_t CDx11Context::PassBeg(I3DPass * pPass)
{
	SafeRelease(m_pPass);
	m_pPass = pPass;
	SafeAddRef(m_pPass);
	m_pPass->PassBeg(this);
	return err_ok;
}

err_t CDx11Context::PassEnd()
{
	if(m_pPass)
		m_pPass->PassEnd();
	SafeRelease(m_pPass);
	return err_ok;
}

err_t CDx11Context::ClearBackBuffer(int_x iIndex, float4 color)
{
	if(!m_pContext)
		return err_nullptr;

	if(0 <= iIndex && iIndex < DX11_RENDER_TARGET_COUNT && m_pBackBuffers[iIndex])
	{
		ID3D11RenderTargetView * pRTV = ((CDx11Texture *)m_pBackBuffers[iIndex])->Dx11GetRTV();
		if(pRTV)
		{
			float_32 clr[4] = {color.af[1], color.af[2], color.af[3], color.af[0]};
			m_pContext->ClearRenderTargetView(pRTV, clr);
		}
	}
	return err_ok;
}

err_t CDx11Context::ClearDepthStencil(bool bDepth, float_32 fDepth, bool bStencil, uint_8 uiStencil)
{
	if(!m_pContext)
		return err_nullptr;

	if(!m_pDepthStencil)
		return err_nullptr;

	uint_32 uiFlags = 0;
	if(bDepth) uiFlags |= D3D11_CLEAR_DEPTH;
	if(bStencil) uiFlags |= D3D11_CLEAR_STENCIL;
	if(uiFlags)
	{
		ID3D11DepthStencilView * pdsv = ((CDx11Texture *)m_pDepthStencil)->Dx11GetDSV();
		m_pContext->ClearDepthStencilView(pdsv, uiFlags, fDepth, uiStencil);
	}
	return err_ok;
}

err_t CDx11Context::UpdateBuffer(I3DBuffer * pBuffer, rectix rect, const void * data, int_x size)
{
	if(!pBuffer || !data || !size)
		return err_invalidarg;

	HRESULT hResult = S_OK;
	ID3D11Buffer * pdx11Buffer = ((CDx11Buffer *)pBuffer)->Dx11GetBuffer();
	if(rect.is_valid())
	{
		D3D11_BOX dxbox;
		dxbox.left = (uint_32)rect.left;
		dxbox.right = (uint_32)rect.right();
		dxbox.top = (uint_32)rect.top;
		dxbox.bottom = (uint_32)rect.bottom();
		dxbox.back = 0;
		dxbox.front = 1;
		m_pContext->UpdateSubresource(pdx11Buffer, 0, &dxbox, data, (uint_32)size, 0);
	}
	else
		m_pContext->UpdateSubresource(pdx11Buffer, 0, nullptr, data, (uint_32)size, 0);
	return err_ok;
}

void CDx11Context::VSSetConstBuffers(int_x iSlotOffset, I3DBuffer ** ppBuffer, int_x iCount)
{
	if(!ppBuffer || iSlotOffset < 0 || iCount < 0)
		return;

	int_x iMaxCount = DX11_EFFECT_MAX_BUFFER_COUNT - iSlotOffset;
	if(iCount > iMaxCount)
		iCount = iMaxCount;

	ID3D11Buffer * buffers[DX11_EFFECT_MAX_BUFFER_COUNT] = {};
	for(int_x cnt = 0; cnt < iCount; ++cnt)
	{
		CDx11Buffer * pDx11Buffer = (CDx11Buffer *)(ppBuffer[cnt]);
		if(!pDx11Buffer)
			return;
		buffers[cnt] = pDx11Buffer->Dx11GetBuffer();
	}

	m_pContext->VSSetConstantBuffers((uint_32)iSlotOffset, (uint_32)iCount, buffers);
}

void CDx11Context::PSSetConstBuffers(int_x iSlotOffset, I3DBuffer ** ppBuffer, int_x iCount)
{
	if(!ppBuffer || iSlotOffset < 0 || iCount < 0)
		return;

	int_x iMaxCount = DX11_EFFECT_MAX_BUFFER_COUNT - iSlotOffset;
	if(iCount > iMaxCount)
		iCount = iMaxCount;

	ID3D11Buffer * buffers[DX11_EFFECT_MAX_BUFFER_COUNT] = {};
	for(int_x cnt = 0; cnt < iCount; ++cnt)
	{
		CDx11Buffer * pDx11Buffer = (CDx11Buffer *)(ppBuffer[cnt]);
		if(!pDx11Buffer)
			return;
		buffers[cnt] = pDx11Buffer->Dx11GetBuffer();
	}

	m_pContext->PSSetConstantBuffers((uint_32)iSlotOffset, (uint_32)iCount, buffers);
}

err_t CDx11Context::SetCBuffer(shader_e eShader, I3DBuffer * pBuffer, int_x iSlot)
{
	if(!m_pContext)
		return err_not_initialized;

	if(!pBuffer)
		(m_pContext->*SetConstBuffersFuns[eShader])((uint_32)iSlot, 0, nullptr);
	else
	{
		ID3D11Buffer * pdx11Buffer = ((CDx11Buffer *)pBuffer)->Dx11GetBuffer();
		if(!pdx11Buffer)
			return err_nullptr;

		(m_pContext->*SetConstBuffersFuns[eShader])((uint_32)iSlot, 1, &pdx11Buffer);
	}
	return err_ok;
}

err_t CDx11Context::SetBuffer(shader_e eShader, I3DBuffer * pBuffer, int_x iSlot)
{
	if(!m_pContext)
		return err_not_initialized;

	if(!pBuffer)
		(m_pContext->*SetShaderResourcesFuns[eShader])((uint_32)iSlot, 0, nullptr);
	else
	{
		CDx11Buffer * pdxBuffer = (CDx11Buffer *)pBuffer;
		ID3D11ShaderResourceView * pView = pdxBuffer->Dx11GetSRV();
		if(!pView)
			return err_nullptr;
		(m_pContext->*SetShaderResourcesFuns[eShader])((uint_32)iSlot, 1, &pView);
	}
	return err_ok;
}

void CDx11Context::SetTexture(shader_e eShader, I3DTexture * pTexture, int_x iSlot)
{
	if(!m_pContext)
		return;

	typedef void(__cdecl ID3D11DeviceContext::*SetShaderResourcesFunT)(UINT, UINT, ID3D11ShaderResourceView * const *);
	const SetShaderResourcesFunT FUNS[shader_num] =
	{
		&ID3D11DeviceContext::VSSetShaderResources,
		&ID3D11DeviceContext::PSSetShaderResources,
		&ID3D11DeviceContext::HSSetShaderResources,
		&ID3D11DeviceContext::DSSetShaderResources,
		&ID3D11DeviceContext::GSSetShaderResources,
		&ID3D11DeviceContext::CSSetShaderResources,
	};

	if(FUNS[eShader] == nullptr)
		return;

	ID3D11ShaderResourceView * pView = nullptr;
	UINT uiNumViews = 0;
	if(pTexture)
	{
		pView = ((CDx11Texture *)pTexture)->Dx11GetSRV();
		uiNumViews = 1;
	}
	else
		uiNumViews = 1;

	(m_pContext->*FUNS[eShader])((uint_32)iSlot, uiNumViews, &pView);
}

void CDx11Context::SetVertexBuffer(I3DBuffer * pvBuffer, vertexformat_e eVertexFormat)
{
	if(!m_pContext)
		return;

	if(m_pPass)
	{
		CDx11Pass * pdxPass = dynamic_cast<CDx11Pass *>(m_pPass);
		if(pdxPass)
			pdxPass->Dx11SetVertexFormat(m_pContext, eVertexFormat);
	}

	if(!pvBuffer)
	{
		UINT uiStride = 0;
		UINT uiOffset = 0;
		m_pContext->IASetVertexBuffers(0, 0, nullptr, &uiStride, &uiOffset);
	}
	else
	{
		CDx11Buffer * pBuffer = (CDx11Buffer *)pvBuffer;
		ID3D11Buffer * pDxVertexBuffer = pBuffer->Dx11GetBuffer();
		//UINT uiStride = (UINT)pvBuffer->GetStride();
		UINT uiStride = (UINT)GetVertexStrideFromFormat(eVertexFormat);
		UINT uiOffset = 0;
		m_pContext->IASetVertexBuffers(0, 1, &pDxVertexBuffer, &uiStride, &uiOffset);
	}
}

void CDx11Context::SetIndexBuffer(I3DBuffer * piBuffer, indexformat_e eIndexFormat)
{
	if(!m_pContext)
		return;

	if(!piBuffer)
	{
		m_pContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
	}
	else
	{
		CDx11Buffer * pBuffer = (CDx11Buffer *)piBuffer;
		ID3D11Buffer * pDxIndexBuffer = pBuffer->Dx11GetBuffer();
		DXGI_FORMAT dIndexFormat = DxgiGetIndexFormat(eIndexFormat);
		m_pContext->IASetIndexBuffer(pDxIndexBuffer, dIndexFormat, 0);
	}
}

void CDx11Context::DrawIndexed(int_x iVertexOffset, int_x iIndexOffset, int_x iIndexCount, primitiveformat_e ePrimitiveFormat)
{
	if(!m_pContext)
		return;

	if(m_pPass)
	{
		CDx11Pass * pdxPass = dynamic_cast<CDx11Pass *>(m_pPass);
		if(pdxPass && pdxPass->Dx11GetHS())
		{
			switch(ePrimitiveFormat)
			{
			case primitiveformat_tri_list:
				ePrimitiveFormat = primitiveformat_tri_patch;
				break;
			default:
				break;
			}
		}
		m_pPass->Flush();
	}

	D3D11_PRIMITIVE_TOPOLOGY dPrimitiveFormat = DxgiGetPrimitiveFormat(ePrimitiveFormat);
	m_pContext->IASetPrimitiveTopology(dPrimitiveFormat);

	m_pContext->DrawIndexed((uint_32)iIndexCount, (uint_32)iIndexOffset, (uint_32)iVertexOffset);
}

void CDx11Context::ClearStates()
{
	if(!m_pContext)
		return;

	ID3D11ShaderResourceView * psrViews[DX11_SRV_COUNT] = {};
	ID3D11RenderTargetView * prtViews[DX11_RENDER_TARGET_COUNT] = {};
	ID3D11DepthStencilView * pdsView = nullptr;
	ID3D11Buffer * pVertexBuffers[DX11_VERTEX_BUFFER_COUNT] = {};
	uint_32 StrideOffset[DX11_VERTEX_BUFFER_COUNT] = {};
	ID3D11Buffer * pConstBuffers[DX11_EFFECT_MAX_BUFFER_COUNT] = {};
	ID3D11SamplerState * pSamplers[DX11_SAMPLER_COUNT] = {};

	// Shaders
	m_pContext->VSSetShader(nullptr, nullptr, 0);
	m_pContext->HSSetShader(nullptr, nullptr, 0);
	m_pContext->DSSetShader(nullptr, nullptr, 0);
	m_pContext->GSSetShader(nullptr, nullptr, 0);
	m_pContext->PSSetShader(nullptr, nullptr, 0);

	// IA
	m_pContext->IASetVertexBuffers(0, DX11_VERTEX_BUFFER_COUNT, pVertexBuffers, StrideOffset, StrideOffset);
	m_pContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
	m_pContext->IASetInputLayout(nullptr);

	// Constant buffers
	m_pContext->VSSetConstantBuffers(0, DX11_EFFECT_MAX_BUFFER_COUNT, pConstBuffers);
	m_pContext->HSSetConstantBuffers(0, DX11_EFFECT_MAX_BUFFER_COUNT, pConstBuffers);
	m_pContext->DSSetConstantBuffers(0, DX11_EFFECT_MAX_BUFFER_COUNT, pConstBuffers);
	m_pContext->GSSetConstantBuffers(0, DX11_EFFECT_MAX_BUFFER_COUNT, pConstBuffers);
	m_pContext->PSSetConstantBuffers(0, DX11_EFFECT_MAX_BUFFER_COUNT, pConstBuffers);

	// Resources
	m_pContext->VSSetShaderResources(0, DX11_SRV_COUNT, psrViews);
	m_pContext->HSSetShaderResources(0, DX11_SRV_COUNT, psrViews);
	m_pContext->DSSetShaderResources(0, DX11_SRV_COUNT, psrViews);
	m_pContext->GSSetShaderResources(0, DX11_SRV_COUNT, psrViews);
	m_pContext->PSSetShaderResources(0, DX11_SRV_COUNT, psrViews);

	// Samplers
	m_pContext->VSSetSamplers(0, DX11_SAMPLER_COUNT, pSamplers);
	m_pContext->HSSetSamplers(0, DX11_SAMPLER_COUNT, pSamplers);
	m_pContext->DSSetSamplers(0, DX11_SAMPLER_COUNT, pSamplers);
	m_pContext->GSSetSamplers(0, DX11_SAMPLER_COUNT, pSamplers);
	m_pContext->PSSetSamplers(0, DX11_SAMPLER_COUNT, pSamplers);

	// Render targets
	m_pContext->OMSetRenderTargets(DX11_RENDER_TARGET_COUNT, prtViews, pdsView);

	// States
	m_pContext->OMSetBlendState(nullptr, DX11_FLOAT_0000, 0xFFFFFFFF);
	m_pContext->OMSetDepthStencilState(nullptr, 0);
	m_pContext->RSSetState(nullptr);

	SafeRelease(m_pPass);
}

VENUS_END
