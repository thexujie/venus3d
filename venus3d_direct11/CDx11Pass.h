#pragma once

#include "Dx11Inc.h"
#include "CDx11Vedio.h"

VENUS_BEG

struct dx11shader_t
{
	ID3DBlob * blob;
	union
	{
		ID3D11DeviceChild * ptr;
		ID3D11VertexShader * vs;
		ID3D11PixelShader * ps;
		ID3D11HullShader * hs;
		ID3D11DomainShader * ds;
		ID3D11GeometryShader * gs;
		ID3D11ComputeShader * cs;
	};
};

struct dx11sampler_t
{
	shader_e shader;
	int_x slot;
	
	sampler_state_t state;
	ID3D11SamplerState * pointer;
};

class CDx11Pass : public ObjectT<I3DPass>
{
public:
	CDx11Pass(CDx11Vedio * pdxVedio);
	~CDx11Pass();

	const char_16 * GetName() const;
	I3DVedio * GetVedio() const;
	I3DEffect * GetEffect() const;
	void SetState(I3DEffect * pEffect, const passstate_t & state);
	void SetSampler(shader_e shader, int_x slot, const sampler_state_t & state);

	void GetShaderDesc(vector<shader_param_t> & params, vector<shader_cbuffer_t> & cbuffers) const;

	err_t PassBeg(I3DContext * pContext);
	err_t Flush();
	err_t PassEnd();

	eff_paramid_t FindParam(const char_16 * name) const;

	err_t SetTexture(eff_paramid_t id, I3DTexture * pTexture);
	err_t SetTexture(const char_16 * name, I3DTexture * pTexture);

	err_t SetBuffer(eff_paramid_t id, I3DBuffer * pBuffer);
	err_t SetBuffer(const char_16 * name, I3DBuffer * pBuffer);

	err_t SetVariable(eff_paramid_t id, const void * data, int_x size);
	err_t SetVariable(const char_16 * name, const void * data, int_x size);

	err_t Dx11SetVertexFormat(ID3D11DeviceContext * pdxContext, vertexformat_e format);

	ID3D11HullShader * Dx11GetHS() const;
protected:
	err_t _CreateObjects();
protected:
	CDx11Vedio * m_pVedio;
	I3DEffect * m_pEffect;

	I3DContext * m_pContext;

	passstate_t m_state;

	linear_map<vertexformat_e, ID3D11InputLayout *> m_ils;
	array<dx11shader_t, shader_num> m_shaders;

	ID3D11RasterizerState * m_pRasterizeState;
	ID3D11DepthStencilState * m_pDepthStencilState;
	ID3D11BlendState * m_pBlendState;
	vector<dx11sampler_t> m_samplers;
};

VENUS_END
