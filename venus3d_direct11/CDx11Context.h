#pragma once

#include "Dx11Inc.h"
#include "CDx11Vedio.h"

VENUS_BEG

typedef void(__cdecl ID3D11DeviceContext::*SetShaderResourcesFunT)(UINT, UINT, ID3D11ShaderResourceView * const *);
const SetShaderResourcesFunT SetShaderResourcesFuns[shader_num] =
{
	&ID3D11DeviceContext::VSSetShaderResources,
	&ID3D11DeviceContext::PSSetShaderResources,
	&ID3D11DeviceContext::HSSetShaderResources,
	&ID3D11DeviceContext::DSSetShaderResources,
	&ID3D11DeviceContext::GSSetShaderResources,
	&ID3D11DeviceContext::CSSetShaderResources,
};

typedef void(__cdecl ID3D11DeviceContext::*SetConstBuffersFunT)(UINT, UINT, ID3D11Buffer * const *);
const SetConstBuffersFunT SetConstBuffersFuns[shader_num] =
{
	&ID3D11DeviceContext::VSSetConstantBuffers,
	&ID3D11DeviceContext::PSSetConstantBuffers,
	&ID3D11DeviceContext::HSSetConstantBuffers,
	&ID3D11DeviceContext::DSSetConstantBuffers,
	&ID3D11DeviceContext::GSSetConstantBuffers,
	&ID3D11DeviceContext::CSSetConstantBuffers,
};


typedef void(__cdecl ID3D11DeviceContext::*SetSamplersFunT)(UINT, UINT, ID3D11SamplerState * const *);
const SetSamplersFunT SetSamplersFuns[shader_num] =
{
	&ID3D11DeviceContext::VSSetSamplers,
	&ID3D11DeviceContext::PSSetSamplers,
	&ID3D11DeviceContext::HSSetSamplers,
	&ID3D11DeviceContext::DSSetSamplers,
	&ID3D11DeviceContext::GSSetSamplers,
	&ID3D11DeviceContext::CSSetSamplers,
};

class CDx11Context : public ObjectT<I3DContext>
{
public:
	CDx11Context(CDx11Vedio * pVedio, ID3D11DeviceContext * pContext);
	~CDx11Context();
	I3DVedio * GetVedio() const;

	void OnDeviceLost();
	void OnDeviceReset();

	void Create();
	void Destroy();

	err_t SetViewPort(rectix viewport);

	err_t SetRenderTarget(I3DTexture ** ppBackBuffer, int_x iCount, I3DTexture * pDepthStencil);
	err_t SetBackBuffer(I3DTexture * pBackBuffer, int_x iIndex = 0);
	err_t SetDepthStencil(I3DTexture * pDepthStencil);
	err_t SetCamera(I3DCamera * pCamera);
	I3DCamera * GetCamera() const;

	err_t PassBeg(I3DPass * pPass);
	err_t PassEnd();

	err_t ClearBackBuffer(int_x iIndex, float4 color);
	err_t ClearDepthStencil(bool bDepth, float_32 fDepth, bool bStencil, uint_8 uiStencil);

	err_t UpdateBuffer(I3DBuffer * pBuffer, rectix rect, const void * data, int_x size);

	void VSSetConstBuffers(int_x iSlotOffset, I3DBuffer ** ppBuffer, int_x iCount);
	void PSSetConstBuffers(int_x iSlotOffset, I3DBuffer ** ppBuffer, int_x iCount);

	err_t SetCBuffer(shader_e eShader, I3DBuffer * pBuffer, int_x iSlot);
	err_t SetBuffer(shader_e eShader, I3DBuffer * pBuffer, int_x iSlot);
	void SetTexture(shader_e eShader, I3DTexture * pTexture, int_x iSlot);

	void SetVertexBuffer(I3DBuffer * pvBuffer, vertexformat_e eVertexFormat);
	void SetIndexBuffer(I3DBuffer * piBuffer, indexformat_e eIndexFormat);
	void DrawIndexed(int_x iVertexOffset, int_x iIndexOffset, int_x iIndexCount, primitiveformat_e ePrimitiveFormat);

public:
	ID3D11DeviceContext * Dx11GetContext() { return m_pContext; }

protected:
	void ClearStates();
protected:
	CDx11Vedio * m_pVedio;
	ID3D11DeviceContext * m_pContext;

	I3DTexture * m_pBackBuffers[DX11_RENDER_TARGET_COUNT];
	I3DTexture * m_pDepthStencil;

	I3DCamera * m_pCamera;
	I3DPass * m_pPass;
};

VENUS_END
