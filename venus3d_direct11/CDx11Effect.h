#pragma once

#include "Dx11Inc.h"
#include "CDx11Vedio.h"

VENUS_BEG

// semantic element
union sign_ele_t
{
	uint_32 value;
	uint_8 text[4];
};

struct dx11_param_t
{
	chbufferw<EFFECT_MAX_IDEITIFIER> name;
	eff_param_type_e type;
	int_x slots[shader_num];
	int_x offsets[shader_num];
	int_x sizes[shader_num];
};

struct cbuffer_variable_index
{
	int_x slot;
	int_x offset;
	int_x size;
};

struct cbuffer_variable
{
	chbuffera<EFFECT_MAX_IDEITIFIER> name;
	cbuffer_variable_index index;
	chbuffera<8> semantic;
};

class CDx11Effect : public ObjectT<I3DEffect>
{
public:
	CDx11Effect(CDx11Vedio * pDevice);
	~CDx11Effect();
	I3DVedio * GetVedio() const;

	const char_16 * GetName() const;

	err_t SetName(const char_16 * szName, int_x iLength = -1);
	err_t SetInputElements(const InputElementT * pElements, int_x iCount);
	err_t SetShader(I3DExplorer * pExplorer, shader_e eShaderType, const char_16 * szFile, const char_16 * szFunction);

	err_t SetSamplerState(shader_e eType, int_x iIndex, const SamplerStateT & state);
	err_t SetPipeLineState(const PipelineStateT & state);

public:
	err_t RenderBeg(I3DContext * pContext);
	err_t Render(I3DContext * pContext);
	err_t RenderEnd(I3DContext * pContext);


	eff_paramid_t FindParam(const char_16 * name) const;

	err_t SetTexture(eff_paramid_t id, I3DTexture * pTexture);
	err_t SetTexture(const char_16 * name, I3DTexture * pTexture);

	err_t SetBuffer(eff_paramid_t id, I3DBuffer * pBuffer);
	err_t SetBuffer(const char_16 * name, I3DBuffer * pBuffer);

	err_t SetCBuffer(eff_paramid_t id, const void * data, int_x size);
	err_t SetCBuffer(const char_16 * name, const void * data, int_x size);
protected:
	dx11_param_t & _TryAddParam(const char_8 * szName);

protected:
	char_16 m_szName[EFFECT_MAX_IDEITIFIER];

	CDx11Vedio * m_pVedio;

	ID3DBlob * m_blobs[shader_num];

	ID3D11InputLayout * m_pInputLayout;
	ID3D11VertexShader * m_pShaderVS;
	ID3D11HullShader * m_pShaderHS;
	ID3D11DomainShader * m_pShaderDS;
	ID3D11PixelShader * m_pShaderPS;

	// cbuffer
	vector<dx11_param_t> m_params;

	array<array<int_x, DX11_EFFECT_MAX_BUFFER_COUNT>, shader_num> m_cbBuffers;
	array<array<ID3D11Buffer *, DX11_EFFECT_MAX_BUFFER_COUNT>, shader_num> m_hwbuffers;
	array<array<uint_8 *, DX11_EFFECT_MAX_BUFFER_COUNT>, shader_num> m_membuffers;
	array<array<bool, DX11_EFFECT_MAX_BUFFER_COUNT>, shader_num> m_resetBuffers;

	vector<cbuffer_variable> m_variables[shader_num];
	SamplerStateT m_samplerStates[shader_num][DX11_SAMPLER_COUNT];
	ID3D11SamplerState * m_samplers[shader_num][DX11_SAMPLER_COUNT];
	bool m_samplerExists[shader_num][DX11_SAMPLER_COUNT];

	PipelineStateT m_state;
	ID3D11RasterizerState * m_pRasterizeState;
	ID3D11BlendState * m_pBlendState;
	ID3D11DepthStencilState * m_pDepthStencilState;

	array<vector<ID3D11ShaderResourceView *>, shader_num> m_srvs;

	float4 m_blendFactor;
	uint_32 m_sampleMask;

	I3DContext * m_pContext;
	//ID3D11DeviceContext * m_pContext;
};

VENUS_END
