#pragma once

#include "DxInc.h"
#include "DxgiInc.h"

#pragma push_macro("new")
#undef new
#include <comdef.h>
#include <d3dcompiler.h>
#include <d3d11.h>
#pragma pop_macro("new")

VENUS_BEG

enum Dx11FeatureE
{
	Dx11Feature_0 = -1,
	Dx11Feature_9_1,
	Dx11Feature_9_2,
	Dx11Feature_9_3,
	Dx11Feature_10_0,
	Dx11Feature_10_1,
	Dx11Feature_11_0,
	Dx11Feature_11_1,
	Dx11FeatureCount,
};

const char DX11_SHADER_PROFILE[shader_num][Dx11FeatureCount][20]
{
	{"vs_4_0_level_9_1", "vs_4_0_level_9_2", "vs_4_0_level_9_3", "vs_4_0", "vs_4_1", "vs_5_0", "vs_5_0"},
	{"ps_4_0_level_9_1", "ps_4_0_level_9_2", "ps_4_0_level_9_3", "ps_4_0", "ps_4_1", "ps_5_0", "ps_5_0"},
	{"hs_4_0_level_9_1", "hs_4_0_level_9_2", "hs_4_0_level_9_3", "hs_4_0", "hs_4_1", "hs_5_0", "hs_5_0"},
	{"ds_4_0_level_9_1", "ds_4_0_level_9_2", "ds_4_0_level_9_3", "ds_4_0", "ds_4_1", "ds_5_0", "ds_5_0"},
};

const GUID GUID_D3DDebugObjectName = {0x429b8c22, 0x9188, 0x4b0c, 0x87, 0x42, 0xac, 0xb0, 0xbf, 0x85, 0xc2, 0x00};
const float_32 DX11_FLOAT_1111[4] = {1.0f, 1.0f, 1.0f, 1.0f};
const float_32 DX11_FLOAT_0000[4] = {0.0f, 0.0f, 0.0f, 0.0f};
const float_32 DX11_FLOAT_1000[4] = {1.0f, 0.0f, 0.0f, 0.0f};
const int_x DX11_VERTEX_BUFFER_COUNT = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
const int_x DX11_EFFECT_MAX_BUFFER_COUNT = minof<int_x>(EFFECT_MAX_CONST_BUFFER, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);

const int_x DX11_SRV_COUNT = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
const int_x DX11_SAMPLER_COUNT = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
const int_x DX11_EFFECT_IA_MAX_ELEMENTS = minof<int_x>(EFFECT_IA_MAX_ELEMENTS, D3D11_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT);
const int_x DX11_RENDER_TARGET_COUNT = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
const int_x DX11_MAX_MIPMAP = 16;
template<typename OT>
void Dx11SetDebugName(OT * pObject, const char_8 * szFormat, ...)
{
	if(pObject)
	{
		char szBuffer[512] = {};
		va_list pArgs = NULL;
		va_start(pArgs, szFormat);
		int_x iLength = textformat_args(szBuffer, 512, szFormat, pArgs);
		va_end(pArgs);

		pObject->SetPrivateData(GUID_D3DDebugObjectName, (uint_32)iLength, szBuffer);
	}
}

template<typename OT>
int_x Dx11GetDebugName(OT * pObject, const char_8 * szName, int_x iSize)
{
	if(pObject)
	{
		uint_32 uiSize = (uint_32)iSize;
		pObject->GetPrivateData(GUID_D3DDebugObjectName, &uiSize, (void *)szName);
		return (int_x)uiSize;
	}
	else
		return 0;
}

void Dx11CleanContext(ID3D11DeviceContext * pContext);

void DumpDx11Device(ID3D11Device * pDevice);
void DumpDx11DeviceContext(ID3D11DeviceContext * pContext);

class Dx11
{
public:
	Dx11() = delete;

	static eff_param_type_e Mapping(D3D_SHADER_VARIABLE_TYPE type);
	static eff_param_type_e Mapping(D3D_SHADER_VARIABLE_CLASS cls, D3D_SHADER_VARIABLE_TYPE type, int_x row, int_x col);

	static D3D11_USAGE Mapping(bufferusage_e value);
	static bufferusage_e Mapping(D3D11_USAGE value);

	static D3D11_CPU_ACCESS_FLAG Mapping(bufferaccess_e value);
	static bufferaccess_e Mapping(D3D11_CPU_ACCESS_FLAG value);

	static D3D11_BIND_FLAG Mapping(bufferbind_e value);
	static bufferbind_e Mapping(D3D11_BIND_FLAG value);

	static D3D11_MAP Mapping(buffermap_e value);
	static D3D11_FILTER Mapping(fitermode_e value);
	static D3D11_TEXTURE_ADDRESS_MODE Mapping(addressmode_e value);
	static D3D11_COMPARISON_FUNC Mapping(compare_e value);
	static D3D11_BLEND Mapping(blendfactor_e value);
	static D3D11_BLEND_OP Mapping(blendop_e value);
	static D3D11_CULL_MODE Mapping(cullmode_e value);
	static D3D11_FILL_MODE Mapping(fillmode_e value);

	static D3D11_STENCIL_OP Mapping(stencilop_e value);

	static uint_8 MappingWriteMask(uint_32 uiMask);
};

int_x Dx11MapElementFormatSize(il_valuetype_e eFormat);
err_t Dx11SaveTextureToDDS(ID3D11DeviceContext * pContext, ID3D11Texture2D * pTexture, const char_16 * szFile);

VENUS_END
