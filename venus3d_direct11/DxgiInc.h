#pragma once

#include "BaseInc.h"
#include <d3dcommon.h>
#include <dxgi.h>

VENUS_BEG

DXGI_FORMAT DxgiMapColorMode(cmode_e eColorMode);
cmode_e DxgiMapColorMode(DXGI_FORMAT eColorMode);

DXGI_FORMAT DxgiGetIndexFormat(indexformat_e eIndexFormat);
DXGI_FORMAT DxgiGetElementFormat(il_valuetype_e eElementFormat);

D3D_PRIMITIVE_TOPOLOGY DxgiGetPrimitiveFormat(primitiveformat_e ePrimitiveFormat);

VENUS_END
