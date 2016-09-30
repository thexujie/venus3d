#pragma once

#include "BaseInc.h"


struct kevertex_t
{
	float3 pos;
	float2 tex;
	quat4 tan;

	static const vertexformat_e FORMAT = vertexformat_pos | vertexformat_tex | vertexformat_tan;
};

struct kemesh_t
{
	vector<kevertex_t> vertices;
	vector<uint_16> indices;

	float3 pos_min;
	float3 pos_max;
	float2 tex_min;
	float2 tex_max;
};

void kemesh_load(kemesh_t & mesh, const char_16 * szFile);

