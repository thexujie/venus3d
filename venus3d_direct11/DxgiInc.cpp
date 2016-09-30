#include "stdafx.h"
#include "DxgiInc.h"

VENUS_BEG

struct dxgi_format_map_t
{
	cmode_e cmode;
	DXGI_FORMAT format;
};

static const dxgi_format_map_t FORMAT_MAPS[] = 
{
	{cmode_r8g8, DXGI_FORMAT_R8G8_UNORM},
	{cmode_a1r5g5b5, DXGI_FORMAT_B5G5R5A1_UNORM},
	{cmode_a8r8g8b8, DXGI_FORMAT_B8G8R8A8_UNORM},
	{cmode_x8r8g8b8, DXGI_FORMAT_B8G8R8A8_UNORM},
	{cmode_a8b8g8r8, DXGI_FORMAT_R8G8B8A8_UNORM},
	{cmode_x8b8g8r8, DXGI_FORMAT_R8G8B8A8_UNORM},
	{cmode_bc1, DXGI_FORMAT_BC1_UNORM},
	{cmode_bc2, DXGI_FORMAT_BC2_UNORM},
	{cmode_bc3, DXGI_FORMAT_BC3_UNORM},
	{cmode_a2r10g10b10, DXGI_FORMAT_R10G10B10A2_UNORM},
	{cmode_a16b16g16r16, DXGI_FORMAT_R16G16B16A16_UNORM},
	{cmode_a16b16g16r16f, DXGI_FORMAT_R16G16B16A16_FLOAT},
	{cmode_b32g32r32f, DXGI_FORMAT_R32G32B32_FLOAT},
	{cmode_a32b32g32r32f, DXGI_FORMAT_R32G32B32A32_FLOAT},
	{cmode_d24s8, DXGI_FORMAT_D24_UNORM_S8_UINT},
};

DXGI_FORMAT DxgiMapColorMode(cmode_e eColorMode)
{
	for(int_x cnt = 0; cnt < sizeof(FORMAT_MAPS) / sizeof(dxgi_format_map_t); ++cnt)
	{
		const dxgi_format_map_t & map = FORMAT_MAPS[cnt];
		if(map.cmode == eColorMode)
			return map.format;
	}
	return DXGI_FORMAT_UNKNOWN;
}

cmode_e DxgiMapColorMode(DXGI_FORMAT eColorMode)
{
	for(int_x cnt = 0; cnt < sizeof(FORMAT_MAPS) / sizeof(dxgi_format_map_t); ++cnt)
	{
		const dxgi_format_map_t & map = FORMAT_MAPS[cnt];
		if(map.format == eColorMode)
			return map.cmode;
	}
	return cmode_invalid;
}

DXGI_FORMAT DxgiGetIndexFormat(indexformat_e eIndexFormat)
{
	switch(eIndexFormat)
	{
	case indexformat_16:
		return DXGI_FORMAT_R16_UINT;
	case indexformat_32:
		return DXGI_FORMAT_R32_UINT;
	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}

DXGI_FORMAT DxgiGetElementFormat(il_valuetype_e eElementFormat)
{
	switch(eElementFormat)
	{
	case il_valuetype_float1:
		return DXGI_FORMAT_R32_FLOAT;
	case il_valuetype_float2:
		return DXGI_FORMAT_R32G32_FLOAT;
	case il_valuetype_float3:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case il_valuetype_float4:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}

D3D_PRIMITIVE_TOPOLOGY DxgiGetPrimitiveFormat(primitiveformat_e ePrimitiveFormat)
{
	switch(ePrimitiveFormat)
	{
	case primitiveformat_tri_list:
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case primitiveformat_tri_strip:
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case primitiveformat_tri_patch:
		return D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
	default:
		break;
	}
	return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

VENUS_END
