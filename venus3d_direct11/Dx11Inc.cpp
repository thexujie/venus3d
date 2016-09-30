#include "stdafx.h"
#include "Dx11Inc.h"

VENUS_BEG

void Dx11CleanContext(ID3D11DeviceContext * pContext)
{
	if(!pContext) 
		return;

	ID3D11ShaderResourceView * psrViews[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	ID3D11RenderTargetView * prtViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
	ID3D11DepthStencilView * pdsView = nullptr;
	ID3D11Buffer * pBuffers[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	ID3D11SamplerState * pSamplers[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint_32 StrideOffset[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	// Shaders
	pContext->VSSetShader(nullptr, nullptr, 0);
	pContext->HSSetShader(nullptr, nullptr, 0);
	pContext->DSSetShader(nullptr, nullptr, 0);
	pContext->GSSetShader(nullptr, nullptr, 0);
	pContext->PSSetShader(nullptr, nullptr, 0);

	// IA clear
	pContext->IASetVertexBuffers(0, 16, pBuffers, StrideOffset, StrideOffset);
	pContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
	pContext->IASetInputLayout(nullptr);

	// Constant buffers
	pContext->VSSetConstantBuffers(0, 14, pBuffers);
	pContext->HSSetConstantBuffers(0, 14, pBuffers);
	pContext->DSSetConstantBuffers(0, 14, pBuffers);
	pContext->GSSetConstantBuffers(0, 14, pBuffers);
	pContext->PSSetConstantBuffers(0, 14, pBuffers);

	// Resources
	pContext->VSSetShaderResources(0, 16, psrViews);
	pContext->HSSetShaderResources(0, 16, psrViews);
	pContext->DSSetShaderResources(0, 16, psrViews);
	pContext->GSSetShaderResources(0, 16, psrViews);
	pContext->PSSetShaderResources(0, 16, psrViews);

	// Samplers
	pContext->VSSetSamplers(0, 16, pSamplers);
	pContext->HSSetSamplers(0, 16, pSamplers);
	pContext->DSSetSamplers(0, 16, pSamplers);
	pContext->GSSetSamplers(0, 16, pSamplers);
	pContext->PSSetSamplers(0, 16, pSamplers);

	// Render targets
	pContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, prtViews, pdsView);

	// States
	pContext->OMSetBlendState(nullptr, DX11_FLOAT_0000, 0xFFFFFFFF);
	pContext->OMSetDepthStencilState(nullptr, 0);
	pContext->RSSetState(nullptr);
}

void DumpDx11Device(ID3D11Device * pDevice)
{

}

void DumpDx11DeviceContext(ID3D11DeviceContext * pContext)
{
	const int_x MAX_COUNT = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
	ID3D11RenderTargetView * prtViews[MAX_COUNT] = {};
	ID3D11DepthStencilView * pdsViews[MAX_COUNT] = {};
	pContext->OMGetRenderTargets(MAX_COUNT, prtViews, pdsViews);

	char_8 szName[512] = {};
	for(int_x cnt = 0; cnt < MAX_COUNT; ++cnt)
	{
		if(prtViews[cnt])
		{
			ID3D11RenderTargetView *& prtView = prtViews[cnt];
			if(Dx11GetDebugName(prtView, szName, 512) >= 0)
				log0("RenderTargetView[%d]:<%s>.", cnt, szName);
			else
				log0("RenderTargetView[%d]:<0x%x>.", cnt, prtView);
			SafeRelease(prtView);
		}
		if(pdsViews[cnt])
		{
			ID3D11DepthStencilView *& pdsView = pdsViews[cnt];
			if(Dx11GetDebugName(pdsView, szName, 512) >= 0)
				log0("DepthStencilView[%d]:<%s>.", cnt, szName);
			else
				log0("DepthStencilView[%d]:<0x%x>.", cnt, pdsView);
			SafeRelease(pdsView);
		}
	}
}

D3D11_USAGE Dx11::Mapping(bufferusage_e eUsage)
{
	switch(eUsage)
	{
	case bufferusage_default:
		return D3D11_USAGE_DEFAULT;
	case bufferusage_dynamic:
		return D3D11_USAGE_DYNAMIC;
	case bufferusage_immutable:
		return D3D11_USAGE_IMMUTABLE;
	case bufferusage_staging:
		return D3D11_USAGE_STAGING;
	default:
		return D3D11_USAGE_DEFAULT;
	}
}

bufferusage_e Dx11::Mapping(D3D11_USAGE eUsage)
{
	switch(eUsage)
	{
	case D3D11_USAGE_DEFAULT:
		return bufferusage_default;
	case D3D11_USAGE_DYNAMIC:
		return bufferusage_dynamic;
	case D3D11_USAGE_IMMUTABLE:
		return bufferusage_immutable;
	case D3D11_USAGE_STAGING:
		return bufferusage_staging;
	default:
		return bufferusage_default;
	}
}

D3D11_CPU_ACCESS_FLAG Dx11::Mapping(bufferaccess_e eAccess)
{
	switch(eAccess)
	{
	case bufferaccess_none:
		return (D3D11_CPU_ACCESS_FLAG)0;
	case bufferaccess_read:
		return D3D11_CPU_ACCESS_READ;
	case bufferaccess_write:
		return D3D11_CPU_ACCESS_WRITE;
	case bufferaccess_readwrite:
		return (D3D11_CPU_ACCESS_FLAG)(D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE);
	default:
		return (D3D11_CPU_ACCESS_FLAG)0;
	}
}

bufferaccess_e Dx11::Mapping(D3D11_CPU_ACCESS_FLAG eAccess)
{
	switch(eAccess)
	{
	case 0:
		return bufferaccess_none;
	case D3D11_CPU_ACCESS_READ:
		return bufferaccess_read;
	case D3D11_CPU_ACCESS_WRITE:
		return bufferaccess_write;
	case D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE:
		return bufferaccess_readwrite;
	default:
		return bufferaccess_none;
	}
}

D3D11_BIND_FLAG Dx11::Mapping(bufferbind_e eBind)
{
	int_x iBind = 0;
	if(eBind & bufferbind_index)
		iBind |= D3D11_BIND_INDEX_BUFFER;
	if(eBind & bufferbind_vertex)
		iBind |= D3D11_BIND_VERTEX_BUFFER;
	if(eBind & bufferbind_const)
		iBind |= D3D11_BIND_CONSTANT_BUFFER;
	if(eBind & bufferbind_resource)
		iBind |= D3D11_BIND_SHADER_RESOURCE;
	if(eBind & bufferbind_rendertarget)
		iBind |= D3D11_BIND_RENDER_TARGET;
	if(eBind & bufferbind_depthstencil)
		iBind |= D3D11_BIND_DEPTH_STENCIL;

	return (D3D11_BIND_FLAG)iBind;
}

D3D11_MAP Dx11::Mapping(buffermap_e eMap)
{
	switch(eMap)
	{
	case buffermap_read:
		return D3D11_MAP_READ;
	case buffermap_write:
		return D3D11_MAP_WRITE;
	case buffermap_readwrite:
		return D3D11_MAP_READ_WRITE;
	case buffermap_write_discard:
		return D3D11_MAP_WRITE_DISCARD;
	case buffermap_write_nooverwrite:
		return D3D11_MAP_WRITE_NO_OVERWRITE;
	default:
		return D3D11_MAP_READ;
	}
}

bufferbind_e Dx11::Mapping(D3D11_BIND_FLAG eBind)
{
	switch(eBind)
	{
	case D3D11_BIND_INDEX_BUFFER:
		return bufferbind_index;
	case D3D11_BIND_VERTEX_BUFFER:
		return bufferbind_vertex;
	case D3D11_BIND_CONSTANT_BUFFER:
		return bufferbind_const;
	case D3D11_BIND_SHADER_RESOURCE:
		return bufferbind_resource;
	case D3D11_BIND_RENDER_TARGET:
		return bufferbind_rendertarget;
	case D3D11_BIND_DEPTH_STENCIL:
		return bufferbind_depthstencil;
	default:
		return bufferbind_none;
	}
}

D3D11_FILTER Dx11::Mapping(fitermode_e eMode)
{
	switch(eMode)
	{
	case fitermode_point:
		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	case fitermode_linear:
		return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	case fitermode_anisotropy:
		return D3D11_FILTER_ANISOTROPIC;
	default:
		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	}
}

D3D11_TEXTURE_ADDRESS_MODE Dx11::Mapping(addressmode_e eMode)
{
	switch(eMode)
	{
	case addressmode_wrap:
		return D3D11_TEXTURE_ADDRESS_WRAP;
	case addressmode_mirror:
		return D3D11_TEXTURE_ADDRESS_MIRROR;
	case addressmode_clamp:
		return D3D11_TEXTURE_ADDRESS_CLAMP;
	case addressmode_border:
		return D3D11_TEXTURE_ADDRESS_BORDER;
	default:
		return D3D11_TEXTURE_ADDRESS_WRAP;
	}
}

D3D11_COMPARISON_FUNC Dx11::Mapping(compare_e eFun)
{
	switch(eFun)
	{
	case compare_never:
		return D3D11_COMPARISON_NEVER;
	case compare_always:
		return D3D11_COMPARISON_ALWAYS;
	case compare_equal:
		return D3D11_COMPARISON_EQUAL;
	case compare_notequal:
		return D3D11_COMPARISON_NOT_EQUAL;
	case compare_less:
		return D3D11_COMPARISON_LESS;
	case compare_lessequal:
		return D3D11_COMPARISON_LESS_EQUAL;
	case compare_greater:
		return D3D11_COMPARISON_GREATER;
	case compare_greaterequal:
		return D3D11_COMPARISON_GREATER_EQUAL;
	default:
		return D3D11_COMPARISON_NEVER;
	}
}

D3D11_BLEND Dx11::Mapping(blendfactor_e eBlend)
{
	switch(eBlend)
	{
	case blendfactor_zero:
		return D3D11_BLEND_ZERO;
	case blendfactor_one:
		return D3D11_BLEND_ONE;
	case blendfactor_srcalpha:
		return D3D11_BLEND_SRC_ALPHA;
	case blendfactor_dstalpha:
		return D3D11_BLEND_DEST_ALPHA;
	case blendfactor_srcalpha_inv:
		return D3D11_BLEND_INV_SRC_ALPHA;
	case blendfactor_dstalpha_inv:
		return D3D11_BLEND_INV_DEST_ALPHA;
	case blendfactor_srccolor:
		return D3D11_BLEND_SRC_COLOR;
	case blendfactor_dstcolor:
		return D3D11_BLEND_DEST_COLOR;
	case blendfactor_srccolor_inv:
		return D3D11_BLEND_INV_SRC_COLOR;
	case blendfactor_dstcolor_inv:
		return D3D11_BLEND_INV_DEST_COLOR;
	case blendfactor_user:
		return D3D11_BLEND_BLEND_FACTOR;
	case blendfactor_user_inv:
		return D3D11_BLEND_INV_BLEND_FACTOR;
	default:
		return D3D11_BLEND_ZERO;
	}
}

D3D11_BLEND_OP Dx11::Mapping(blendop_e eMode)
{
	switch(eMode)
	{
	case blendop_add:
		return D3D11_BLEND_OP_ADD;
	case blendop_sub:
		return D3D11_BLEND_OP_SUBTRACT;
	case blendop_revert_sub:
		return D3D11_BLEND_OP_REV_SUBTRACT;
	case blendop_min:
		return D3D11_BLEND_OP_MIN;
	case blendop_max:
		return D3D11_BLEND_OP_MAX;
	default:
		return D3D11_BLEND_OP_ADD;
	}
}

uint_8 Dx11::MappingWriteMask(uint_32 uiMask)
{
	uint_8 uiRet = 0;
	if(uiMask & 0xFF000000)
		uiRet |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
	if(uiMask & 0x00FF0000)
		uiRet |= D3D11_COLOR_WRITE_ENABLE_RED;
	if(uiMask & 0x0000FF00)
		uiRet |= D3D11_COLOR_WRITE_ENABLE_GREEN;
	if(uiMask & 0x000000FF)
		uiRet |= D3D11_COLOR_WRITE_ENABLE_BLUE;
	return uiRet;
}

D3D11_CULL_MODE Dx11::Mapping(cullmode_e eMode)
{
	switch(eMode)
	{
	case cullmode_none:
		return D3D11_CULL_NONE;
	case cullmode_front:
		return D3D11_CULL_FRONT;
	case cullmode_back:
		return D3D11_CULL_BACK;
	default:
		return D3D11_CULL_NONE;
	}
}

D3D11_FILL_MODE Dx11::Mapping(fillmode_e eMode)
{
	switch(eMode)
	{
	case fillmode_solid:
		return D3D11_FILL_SOLID;
	case fillmode_frame:
		return D3D11_FILL_WIREFRAME;
	default:
		return D3D11_FILL_SOLID;
	}
}

int_x Dx11MapElementFormatSize(il_valuetype_e eFormat)
{
	switch(eFormat)
	{
	case il_valuetype_float1:
		return 4;
	case il_valuetype_float2:
		return 8;
	case il_valuetype_float3:
		return 12;
	case il_valuetype_float4:
		return 16;
	default:
		return 0;
	}
}

err_t Dx11SaveTextureToDDS(ID3D11DeviceContext * pContext, ID3D11Texture2D * pTexture, const char_16 * szFile)
{
	if(!pContext || !pTexture || !szFile)
		return err_invalidarg;

	HRESULT hResult = S_OK;
	D3D11_TEXTURE2D_DESC desc = {};
	pTexture->GetDesc(&desc);

	if(desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ)
	{
		D3D11_MAPPED_SUBRESOURCE subRes = {};
		hResult = pContext->Map(pTexture, 0, D3D11_MAP_READ, 0, &subRes);
		if(hResult == S_OK)
		{
			pContext->Unmap(pTexture, 0);
		}
		SafeRelease(pContext);
	}
	else
	{
		ID3D11Device * pDevice = nullptr;
		pContext->GetDevice(&pDevice);
		if(!pDevice)
			return err_failed;

		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BindFlags = 0;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		ID3D11Texture2D * pStaging = nullptr;
		hResult = pDevice->CreateTexture2D(&desc, nullptr, &pStaging);
		if(!pStaging)
		{
			SafeRelease(pDevice);
			SafeRelease(pContext);
			return err_failed;
		}

		//pContext->ResolveSubresource(pStaging, 0, pTexture, 0, desc.Format);
		pContext->CopyResource(pStaging, pTexture);
		D3D11_MAPPED_SUBRESOURCE subRes = {};
		hResult = pContext->Map(pStaging, 0, D3D11_MAP_READ, 0, &subRes);
		if(hResult == S_OK)
		{
			pContext->Unmap(pStaging, 0);
		}

		// TODO£ºSave texture.
		//hResult = D3DX11SaveTextureToFileW(pContext, pStaging, D3DX11_IFF_DDS, szFile);
		SafeRelease(pStaging);
		SafeRelease(pDevice);
		SafeRelease(pContext);
	}
	return err_failed;
}

eff_param_type_e Dx11::Mapping(D3D_SHADER_VARIABLE_TYPE type)
{
	switch(type)
	{
	case D3D10_SVT_VOID:
		return eff_param_type_type_user;
	case D3D10_SVT_BOOL:
		return eff_param_type_type_bool;
	case D3D10_SVT_INT:
		return eff_param_type_type_int;
	case D3D10_SVT_FLOAT:
		return eff_param_type_type_float;
	default:
		return eff_param_type_none;
	}
}
eff_param_type_e Dx11::Mapping(D3D_SHADER_VARIABLE_CLASS cls, D3D_SHADER_VARIABLE_TYPE type, int_x row, int_x col)
{
	if(cls == D3D_SVC_STRUCT)
		return eff_param_type_struct;

	return (eff_param_type_e)eff_param_type_make(Dx11::Mapping(type), (int_32)row, (int_32)col);
}

D3D11_STENCIL_OP Dx11::Mapping(stencilop_e value)
{
	switch(value)
	{
	case stencilop_none: return D3D11_STENCIL_OP_KEEP;
	case stencilop_zero: return D3D11_STENCIL_OP_ZERO;
	case stencilop_replace: return D3D11_STENCIL_OP_REPLACE;
	case stencilop_inc_clamp: return D3D11_STENCIL_OP_INCR_SAT;
	case stencilop_dec_clamp: return D3D11_STENCIL_OP_DECR_SAT;
	case stencilop_inc_wrap: return D3D11_STENCIL_OP_INCR;
	case stencilop_dec_wrap: return D3D11_STENCIL_OP_DECR;
	case stencilop_invert: return D3D11_STENCIL_OP_INVERT;
	default: return D3D11_STENCIL_OP_KEEP;
	}
}
VENUS_END
