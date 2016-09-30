#pragma once

#include "BaseInc.h"

struct vemesh_header_t
{
	uint_32 size;
	uint_32 magic;

	vertexformat_e vertexformat;
	indexformat_e indexformat;
	primitiveformat_e primitiveformat;

	int_32 num_vertices;
	int_32 num_indices;
	int_32 num_nodes;
};

struct vescene_header_t
{
	uint_32 size;
	uint_32 magic;
	
};
