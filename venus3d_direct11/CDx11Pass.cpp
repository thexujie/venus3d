#include "stdafx.h"
#include "CDx11Pass.h"
#include "CDx11Context.h"

VENUS_BEG

CDx11Pass::CDx11Pass(CDx11Vedio * pdxVedio) :
	m_pVedio(pdxVedio), m_pEffect(nullptr),
	m_pRasterizeState(nullptr), m_pDepthStencilState(nullptr), m_pBlendState(nullptr)
{
	SafeAddRef(m_pVedio);
	buffclr(m_shaders);
}

CDx11Pass::~CDx11Pass()
{
	for(pair<vertexformat_e, ID3D11InputLayout *> & pair : m_ils) SafeRelease(pair.value);
	m_ils.clear();

	for(dx11sampler_t & sampler : m_samplers) SafeRelease(sampler.pointer);
	m_samplers.clear();

	for(dx11shader_t & shader : m_shaders) { SafeRelease(shader.blob); SafeRelease(shader.ptr); }

	SafeRelease(m_pRasterizeState);
	SafeRelease(m_pDepthStencilState);
	SafeRelease(m_pBlendState);
	SafeRelease(m_pVedio);
	FakeRelease(m_pEffect);
}

I3DVedio * CDx11Pass::GetVedio() const
{
	return m_pVedio;
}

const char_16 * CDx11Pass::GetName() const
{
	return m_state.name;
}

I3DEffect * CDx11Pass::GetEffect() const
{
	return m_pEffect;
}

void CDx11Pass::SetState(I3DEffect * pEffect, const passstate_t & state)
{
	SafeRelease(m_pRasterizeState);
	SafeRelease(m_pDepthStencilState);
	SafeRelease(m_pBlendState);

	for(pair<vertexformat_e, ID3D11InputLayout *> & pair : m_ils) SafeRelease(pair.value);
	m_ils.clear();

	for(dx11sampler_t & sampler : m_samplers) SafeRelease(sampler.pointer);
	m_samplers.clear();

	SafeRelease(m_pEffect);
	m_pEffect = pEffect;
	FakeAddRef(m_pEffect);
	m_state = state;
	_CreateObjects();
	return;
}

void CDx11Pass::SetSampler(shader_e shader, int_x slot, const sampler_state_t & state)
{
	int_x iIndex = -1;
	for(int_x cnt = 0; cnt < m_samplers.size(); ++cnt)
	{
		dx11sampler_t & dxsampler = m_samplers[cnt];
		if(dxsampler.shader == shader && dxsampler.slot == slot)
		{
			SafeRelease(dxsampler.pointer);
			iIndex = cnt;
			break;
		}
	}

	if(iIndex < 0)
	{
		iIndex = m_samplers.size();
		dx11sampler_t & dxsampler = m_samplers.add();
		dxsampler.shader = shader;
		dxsampler.slot = slot;
		dxsampler.pointer = nullptr;
	}

	dx11sampler_t & dxsampler = m_samplers[iIndex];
	dxsampler.state = state;
}

static void TryAddResource(vector<shader_param_t> & params,
	const char_8 * name_param, eff_param_type_e type, int_x shader, int_x slot, int_x size = 0)
{
	chbufferw<EFFECT_MAX_IDEITIFIER> namew;
	ansitounicode(name_param, -1, namew.buffer, EFFECT_MAX_IDEITIFIER);

	int_x index_param = -1;
	int_x index_cbuffer = -1;
	for(int_x cnt = 0; cnt < params.size(); ++cnt)
	{
		if(params[cnt].name == namew)
		{
			index_param = cnt;
			break;
		}
	}

	if(index_param < 0)
	{
		shader_param_t & param = params.add();
		param.name = namew;
		param.type = type;
		param.size = size;
		arrayfill(param.slots, (int_x)-1);
		arrayfill(param.offsets, (int_x)-1);
		index_param = params.size() - 1;
	}
	else {}

	shader_param_t & param = params[index_param];
	if(param.size != size)
	{
		log2(L"%S param.size != %d.", name_param, size);
		param.size = 1;
	}
	if(param.type != type)
	{
		log2(L"%S param.type != type.", name_param);
		param.type = type;
	}

	if(param.slots[shader] >= 0)
	{
		log2(L"%S, param.slots[%d] have been used.", name_param, shader);
	}
	else
	{
		param.slots[shader] = slot;
		param.offsets[shader] = 0;
	}
}

static void TryAddParam(vector<shader_param_t> & params, vector<shader_cbuffer_t> & cbuffers,
	const char_8 * name_cbuffer,
	const char_8 * name_param, eff_param_type_e type, int_x shader, int_x slot, int_x offset, int_x size)
{
	chbufferw<EFFECT_MAX_IDEITIFIER> namew;
	ansitounicode(name_param, -1, namew.buffer, EFFECT_MAX_IDEITIFIER);

	int_x index_param = -1;
	int_x index_cbuffer = -1;
	for(int_x cnt = 0; cnt < params.size(); ++cnt)
	{
		if(params[cnt].name == namew)
		{
			index_param = cnt;
			break;
		}
	}

	for(int_x cnt = 0; cnt < cbuffers.size(); ++cnt)
	{
		if(cbuffers[cnt].shader == shader && cbuffers[cnt].slot == slot)
		{
			index_cbuffer = cnt;
			break;
		}
	}

	if(index_param < 0)
	{
		shader_param_t & param = params.add();
		param.name = namew;
		param.type = type;
		param.size = size;
		arrayfill(param.slots, (int_x)-1);
		arrayfill(param.offsets, (int_x)-1);
		index_param = params.size() - 1;
	}
	else {}

	if(index_cbuffer < 0)
	{
		chbufferw<EFFECT_MAX_IDEITIFIER> cbuffer_namew;
		ansitounicode(name_cbuffer, -1, cbuffer_namew.buffer, EFFECT_MAX_IDEITIFIER);
		shader_cbuffer_t & cbuffer = cbuffers.add();
		cbuffer.name = cbuffer_namew;
		cbuffer.shader = (shader_e)shader;
		cbuffer.slot = slot;
		cbuffer.size = 0;
		index_cbuffer = cbuffers.size() - 1;
	}
	else {}

	shader_param_t & param = params[index_param];
	shader_cbuffer_t & cbuffer = cbuffers[index_cbuffer];

	if(param.size != size)
	{
		log2(L"%S.%S param.size != size.", name_cbuffer, name_param);
		param.size = maxof(param.size, size);
	}
	if(param.type != type)
	{
		log2(L"%S.%S param.type != type.", name_cbuffer, name_param);
		param.type = type;
	}

	if(param.slots[shader] >= 0)
	{
		log2(L"%S.%S param.buffer[%d] have been used.", name_cbuffer, name_param, shader);
	}
	else
	{
		param.slots[shader] = slot;
		param.offsets[shader] = offset;

		if(offset + size > cbuffer.size)
			cbuffer.size = offset + size;
	}
}

void CDx11Pass::GetShaderDesc(vector<shader_param_t> & params, vector<shader_cbuffer_t> & cbuffers) const
{
	winerr_t err = S_OK;
	D3D11_SHADER_DESC shaderDesc = {};
	int_x iBufferIndex = 0;
	int_x iSamplerIndex = 0;
	int_x iSlotIndex = 0;
	D3D11_SHADER_BUFFER_DESC cbufferDesc = {};
	D3D11_SHADER_INPUT_BIND_DESC bindDesc = {};
	D3D11_SHADER_VARIABLE_DESC variableDesc = {};
	D3D11_SHADER_TYPE_DESC typeDesc = {};
	D3D11_SIGNATURE_PARAMETER_DESC signDesc = {};

	for(int_x ishader = 0; ishader < shader_num; ++ishader)
	{
		const dx11shader_t & dx11shader = m_shaders[ishader];
		if(!dx11shader.blob)
			continue;

		ID3D11ShaderReflection * pReflect = nullptr;
		err = m_pVedio->DxReflect((const byte_t *)dx11shader.blob->GetBufferPointer(), (int_x)dx11shader.blob->GetBufferSize(),
			__uuidof(ID3D11ShaderReflection), (void **)&pReflect);
		if(err.failed())
		{
			log2(L"Can not get ID3D11ShaderReflection interface of %s.", SHADER_NAME[ishader]);
			continue;
		}

		pReflect->GetDesc(&shaderDesc);
		// maybe use to generate the functional input layout.
		for(uint_32 iparam = 0; iparam < shaderDesc.InputParameters; ++iparam)
		{
			pReflect->GetInputParameterDesc((uint_32)iparam, &signDesc);
			signDesc.Mask = 0;
		}

		for(uint_32 ires = 0; ires < shaderDesc.BoundResources; ++ires)
		{
			pReflect->GetResourceBindingDesc(ires, &bindDesc);
			iSlotIndex = (int_x)bindDesc.BindPoint;
			// buffer.
			if(bindDesc.Type == D3D_SIT_CBUFFER)
			{
				ID3D11ShaderReflectionConstantBuffer * pcbuffer = pReflect->GetConstantBufferByName(bindDesc.Name);
				if(!pcbuffer)
					continue;

				pcbuffer->GetDesc(&cbufferDesc);
				log(4, L"%s(%s): cbuffer %S(regieter:%d) %d bytes.", m_state.shader.buffer, m_state.main[ishader],
					cbufferDesc.Name, bindDesc.BindPoint, cbufferDesc.Size);
				for(uint_32 ivar = 0; ivar < cbufferDesc.Variables; ++ivar)
				{
					ID3D11ShaderReflectionVariable * pshdrefVariable = pcbuffer->GetVariableByIndex(ivar);
					if(!pshdrefVariable)
						continue;

					ID3D11ShaderReflectionType * pshdrefType = pshdrefVariable->GetType();
					if(!pshdrefType)
						continue;

					pshdrefVariable->GetDesc(&variableDesc);
					pshdrefType->GetDesc(&typeDesc);

					eff_param_type_e type = Dx11::Mapping(typeDesc.Class, typeDesc.Type, (int_32)typeDesc.Rows, (int_32)typeDesc.Columns);

					TryAddParam(params, cbuffers,
						cbufferDesc.Name, variableDesc.Name, type,
						ishader, bindDesc.BindPoint, variableDesc.StartOffset, variableDesc.Size);
					log0("  %s.%s %d bytes.", cbufferDesc.Name, variableDesc.Name, variableDesc.Size);

					SafeNull(pshdrefType);
					SafeNull(pshdrefVariable);
				}
				SafeNull(pcbuffer);
			}
			else if(bindDesc.Type == D3D_SIT_TEXTURE)
			{
				eff_param_type_e type = eff_param_type_none;
				switch(bindDesc.Dimension)
				{
				case D3D_SRV_DIMENSION_TEXTURE1D: type = eff_param_type_texture_1d; break;
				case D3D_SRV_DIMENSION_TEXTURE2D: type = eff_param_type_texture_2d; break;
				case D3D_SRV_DIMENSION_BUFFER: type = eff_param_type_buffer; break;
				default:
					break;
				}
				if(type != eff_param_type_none)
					TryAddResource(params, bindDesc.Name, type, ishader, bindDesc.BindPoint);
			}
			else if(bindDesc.Type == D3D_SIT_SAMPLER)
			{
				eff_param_type_e type = eff_param_type_sampler;
				TryAddResource(params, bindDesc.Name, type, ishader, bindDesc.BindPoint);
			}
			else
			{
			}
		}

	}

}

err_t CDx11Pass::_CreateObjects()
{
	if(!m_pVedio)
		return err_not_initialized;

	if(m_state.shader.is_empty())
		return err_bad_data;

	HRESULT hResult = S_OK;
	ID3D11Device * pDxDevice = m_pVedio->Dx11GetDevice();

	for(pair<vertexformat_e, ID3D11InputLayout *> & pair : m_ils) SafeRelease(pair.value);
	m_ils.clear();

	ID3D10Blob * pError = nullptr;
	uint_32 uiFlag1 = 0;
	uiFlag1 |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#ifdef _DEBUG
	uiFlag1 |= D3DCOMPILE_DEBUG;
	uiFlag1 |= D3DCOMPILE_SKIP_OPTIMIZATION;
	uiFlag1 |= D3DCOMPILE_OPTIMIZATION_LEVEL0;
#endif

	for(int_x ishader = 0; ishader < shader_num; ++ishader)
	{
		if(m_state.main[ishader].is_empty())
			continue;

		dx11shader_t & dx11shader = m_shaders[ishader];

		hResult = m_pVedio->DxCompile((shader_e)ishader,
			m_state.shader, m_state.main[ishader],
			uiFlag1, 0,
			&dx11shader.blob, &pError);

		if(pError)
		{
			log1(L"%s code compile error : \n%S", SHADER_NAME[ishader], pError->GetBufferPointer());
			SafeRelease(pError);
		}

		if(!dx11shader.blob)
			continue;

		switch(ishader)
		{
		case shader_vs:
			hResult = pDxDevice->CreateVertexShader(
				dx11shader.blob->GetBufferPointer(), dx11shader.blob->GetBufferSize(),
				nullptr, &dx11shader.vs);
			break;
		case shader_ps:
			hResult = pDxDevice->CreatePixelShader(
				dx11shader.blob->GetBufferPointer(), dx11shader.blob->GetBufferSize(),
				nullptr, &dx11shader.ps);
			break;
		case shader_hs:
			hResult = pDxDevice->CreateHullShader(
				dx11shader.blob->GetBufferPointer(), dx11shader.blob->GetBufferSize(),
				nullptr, &dx11shader.hs);
			break;
		case shader_ds:
			hResult = pDxDevice->CreateDomainShader(
				dx11shader.blob->GetBufferPointer(), dx11shader.blob->GetBufferSize(),
				nullptr, &dx11shader.ds);
			break;
		case shader_gs:
			hResult = pDxDevice->CreateGeometryShader(
				dx11shader.blob->GetBufferPointer(), dx11shader.blob->GetBufferSize(),
				nullptr, &dx11shader.gs);
			break;
		case shader_cs:
			hResult = pDxDevice->CreateComputeShader(
				dx11shader.blob->GetBufferPointer(), dx11shader.blob->GetBufferSize(),
				nullptr, &dx11shader.cs);
			break;
		default:
			break;
		}

		if(hResult)
		{
			log1(L"%s create error : %s", SHADER_NAME[ishader], Win32::FormatWinError(hResult));
			SafeRelease(pError);
		}
	}

	return err_ok;
}


err_t CDx11Pass::PassBeg(I3DContext * pContext)
{
	m_pEffect->PassBeg(pContext);

	HRESULT hResult = S_OK;
	ID3D11Device * pDxDevice = m_pVedio->Dx11GetDevice();

	CDx11Context * pdx11Context = dynamic_cast<CDx11Context*>(pContext);
	if(!pdx11Context)
		return err_invalidcall;

	ID3D11DeviceContext * pdxContext = pdx11Context->Dx11GetContext();
	if(!pdx11Context)
		return err_invalidcall;

	m_pContext = pContext;
	SafeAddRef(m_pContext);

	if(!m_pRasterizeState)
	{
		D3D11_RASTERIZER_DESC ras_desc = {};
		ras_desc.AntialiasedLineEnable = FALSE;
		ras_desc.CullMode = Dx11::Mapping(m_state.cullmode);

		ras_desc.DepthBias = (int_32)m_state.z_bias;
		ras_desc.DepthBiasClamp = m_state.z_bias_clamp;
		ras_desc.SlopeScaledDepthBias = m_state.z_bias_slope_scaled;

		ras_desc.DepthClipEnable = m_state.z_clip;
		ras_desc.FillMode = Dx11::Mapping(m_state.fillmode);
		ras_desc.FrontCounterClockwise = m_state.clockwise;
		ras_desc.MultisampleEnable = m_state.multisampler;
		ras_desc.ScissorEnable = m_state.scissor;
		hResult = pDxDevice->CreateRasterizerState(&ras_desc, &m_pRasterizeState);
	}

	if(!m_pDepthStencilState)
	{
		D3D11_DEPTH_STENCIL_DESC ds_desc = {};
		ds_desc.DepthEnable = m_state.depth_test;
		ds_desc.DepthWriteMask = m_state.depth_write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		ds_desc.DepthFunc = Dx11::Mapping(m_state.depth_cmp);
		ds_desc.StencilEnable = m_state.stencil_test;
		ds_desc.StencilReadMask = m_state.stencil_read;
		ds_desc.StencilWriteMask = m_state.stencil_write;
		ds_desc.FrontFace.StencilFunc = Dx11::Mapping(m_state.stencil_front_cmp);
		ds_desc.FrontFace.StencilFailOp = Dx11::Mapping(m_state.stencil_front_op_fail);
		ds_desc.FrontFace.StencilPassOp = Dx11::Mapping(m_state.stencil_front_op_pass);
		ds_desc.FrontFace.StencilDepthFailOp = Dx11::Mapping(m_state.stencil_front_op_depth_fail);
		ds_desc.BackFace.StencilFunc = Dx11::Mapping(m_state.stencil_back_cmp);
		ds_desc.BackFace.StencilFailOp = Dx11::Mapping(m_state.stencil_back_op_fail);
		ds_desc.BackFace.StencilPassOp = Dx11::Mapping(m_state.stencil_back_op_pass);
		ds_desc.BackFace.StencilDepthFailOp = Dx11::Mapping(m_state.stencil_back_op_depth_fail);
		hResult = pDxDevice->CreateDepthStencilState(&ds_desc, &m_pDepthStencilState);
	}

	if(!m_pBlendState)
	{
		D3D11_BLEND_DESC blend_desc = {};
		blend_desc.IndependentBlendEnable = FALSE;
		blend_desc.AlphaToCoverageEnable = m_state.blend_alpha2coverage;
		blend_desc.RenderTarget[0].BlendEnable = m_state.blend_enable;
		blend_desc.RenderTarget[0].SrcBlend = Dx11::Mapping(m_state.blend_color_src);
		blend_desc.RenderTarget[0].DestBlend = Dx11::Mapping(m_state.blend_color_dst);
		blend_desc.RenderTarget[0].BlendOp = Dx11::Mapping(m_state.blend_color_op);

		blend_desc.RenderTarget[0].SrcBlendAlpha = Dx11::Mapping(m_state.blend_alpha_src);
		blend_desc.RenderTarget[0].DestBlendAlpha = Dx11::Mapping(m_state.blend_alpha_dst);
		blend_desc.RenderTarget[0].BlendOpAlpha = Dx11::Mapping(m_state.blend_alpha_op);
		blend_desc.RenderTarget[0].RenderTargetWriteMask = Dx11::MappingWriteMask(m_state.blend_write_mask);
		hResult = pDxDevice->CreateBlendState(&blend_desc, &m_pBlendState);
	}

	for(dx11sampler_t & sampler : m_samplers)
	{
		if(!sampler.pointer)
		{
			D3D11_SAMPLER_DESC samplerDesc = {};
			samplerDesc.MinLOD = F32_MIN;
			samplerDesc.MaxLOD = F32_MIN;
			samplerDesc.MipLODBias = 0.0f;

			samplerDesc.Filter = Dx11::Mapping(sampler.state.filtermode);
			samplerDesc.AddressU = Dx11::Mapping(sampler.state.addressmode_u);
			samplerDesc.AddressV = Dx11::Mapping(sampler.state.addressmode_v);
			samplerDesc.AddressW = Dx11::Mapping(sampler.state.addressmode_w);
			samplerDesc.MaxAnisotropy = (uint_32)sampler.state.anisotropy;
			samplerDesc.ComparisonFunc = Dx11::Mapping(sampler.state.cmp);
			hResult = pDxDevice->CreateSamplerState(&samplerDesc, &sampler.pointer);
			Dx11SetDebugName(sampler.pointer, "ID3D11SamplerState");
		}
		else {}

		(pdxContext->*SetSamplersFuns[sampler.shader])((uint_32)sampler.slot, 1, &sampler.pointer);
	}

	pdxContext->VSSetShader(m_shaders[shader_vs].vs, nullptr, 0);
	pdxContext->PSSetShader(m_shaders[shader_ps].ps, nullptr, 0);
	pdxContext->HSSetShader(m_shaders[shader_hs].hs, nullptr, 0);
	pdxContext->DSSetShader(m_shaders[shader_ds].ds, nullptr, 0);
	pdxContext->GSSetShader(m_shaders[shader_gs].gs, nullptr, 0);
	pdxContext->CSSetShader(m_shaders[shader_cs].cs, nullptr, 0);

	pdxContext->RSSetState(m_pRasterizeState);
	pdxContext->OMSetDepthStencilState(m_pDepthStencilState, m_state.stencil_ref);
	pdxContext->OMSetBlendState(m_pBlendState, m_state.blend_factor, m_state.blend_sample_mask);
	return err_ok;
}

err_t CDx11Pass::Flush()
{
	m_pEffect->Flush();
	return err_ok;
}

err_t CDx11Pass::PassEnd()
{
	m_pEffect->PassEnd();
	SafeRelease(m_pContext);
	return err_ok;
}

eff_paramid_t CDx11Pass::FindParam(const char_16 * name) const
{
	return m_pEffect->FindParam(name);
}

err_t CDx11Pass::SetTexture(eff_paramid_t id, I3DTexture * pTexture)
{
	return m_pEffect->SetTexture(id, pTexture);
}
err_t CDx11Pass::SetTexture(const char_16 * name, I3DTexture * pTexture)
{
	return m_pEffect->SetTexture(name, pTexture);
}

err_t CDx11Pass::SetBuffer(eff_paramid_t id, I3DBuffer * pBuffer)
{
	return m_pEffect->SetBuffer(id, pBuffer);
}

err_t CDx11Pass::SetBuffer(const char_16 * name, I3DBuffer * pBuffer)
{
	return m_pEffect->SetBuffer(name, pBuffer);
}

err_t CDx11Pass::SetVariable(eff_paramid_t id, const void * data, int_x size)
{
	return m_pEffect->SetVariable(id, data, size);
}

err_t CDx11Pass::SetVariable(const char_16 * name, const void * data, int_x size)
{
	return m_pEffect->SetVariable(name, data, size);
}

err_t CDx11Pass::Dx11SetVertexFormat(ID3D11DeviceContext * pdxContext, vertexformat_e format)
{
	ID3D11InputLayout *& pil = m_ils[format];
	if(!pil)
	{
		struct _mapping_t
		{
			vertexformat_e vformat;
			char_8 semantic[EFFECT_MAX_IDEITIFIER];
			int_x index;
			DXGI_FORMAT format;
			int_x size;
		};
		struct vertex_ter_t
		{
			float3 pos;
			float2 tex;
			// 法线
			float3 nom;
			// 副法线
			float3 bin;
			// 切线
			float3 tan;
		};
		const _mapping_t MAPPINGS[] =
		{
			{vertexformat_pos, "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 12},
			{vertexformat_nor, "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 12},
			{vertexformat_tex, "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 8},
			{vertexformat_tan, "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 16},
			{vertexformat_wgt, "WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 16},
		};

		HRESULT hResult = S_OK;
		ID3D11Device * pDxDevice = m_pVedio->Dx11GetDevice();

		ID3DBlob * blob = m_shaders[shader_vs].blob;
		if(!pDxDevice || !blob)
			return err_nullptr;

		int_x iele = 0;
		int_x offset = 0;
		D3D11_INPUT_ELEMENT_DESC eleDescs[DX11_EFFECT_IA_MAX_ELEMENTS] = {};
		for(int_x cnt = 0; cnt < arraysize(MAPPINGS); ++cnt)
		{
			const _mapping_t & mapping = MAPPINGS[cnt];
			if(!(format & mapping.vformat))
				continue;

			D3D11_INPUT_ELEMENT_DESC & desc = eleDescs[iele++];

			desc.SemanticName = mapping.semantic;
			desc.SemanticIndex = (uint_32)mapping.index;
			desc.Format = mapping.format;
			desc.InputSlot = 0;
			desc.AlignedByteOffset = (uint_32)offset;
			desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			desc.InstanceDataStepRate = 0;
			offset += mapping.size;
		}

		hResult = pDxDevice->CreateInputLayout(eleDescs, (uint_32)iele,
			blob->GetBufferPointer(), blob->GetBufferSize(), &pil);

		if(hResult)
		{
			log2(L"can not create a input layout in %s with vertext format %d.(%s)", m_state.shader, format, Win32::FormatWinError(hResult));
		}
	}
	else {}

	pdxContext->IASetInputLayout(pil);
	return err_ok;
}

ID3D11HullShader * CDx11Pass::Dx11GetHS() const
{
	return m_shaders[shader_hs].hs;
}

VENUS_END

