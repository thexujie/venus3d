#pragma once

#include "BaseInc.h"

namespace exporter
{

enum texture_type_e
{
	texture_type_environment,
	texture_type_diffuse,
	texture_type_specular,
	texture_type_specular_level,
	texture_type_shininess,
	texture_type_illumination,
	texture_type_opacity,
	texture_type_filter,
	texture_type_bump,
	texture_type_reflect,
	texture_type_refract,
	texture_type_displacement,
};

const int_x WEIGHT_MAX = 64;

typedef int_x mtlid_t;
typedef int_x smid_t;

struct bonebind_t
{
	int_x index;
	float1 weight;
};

struct node_t
{
	int_x vertex_offset;
	int_x vertex_count;
	int_x index_offset;
	int_x index_count;
};

struct position_t
{
	float3 point;
	array<float1, WEIGHT_MAX> weights;
};

struct vertex_t
{
	position_t position;
	float3 normal;
	float3 tangent;
	float2 texcoord;
};


struct maxtexmap_t
{
	texture_type_e type;
	textx type_name;

	int_x channel;
	Matrix3 trans;
	int3 uvwindex;

	textx mapname;
};

class MaxMtl
{
public:
	virtual ~MaxMtl() {}
	virtual err_t Initialize(Mtl & mtl);

	Mtl * pmtl;
	float3 ambient;
	float3 diffuse;
	float3 specular;
	float3 emit;
	float1 opacity;
	float1 specular_level;
	float1 shininess;

	vector<maxtexmap_t> texmaps;

	// 获取漫反射的贴图通道
	int_x GetDiffuseTexChannel() const;
	const maxtexmap_t & FindTexmap(texture_type_e type) const;
};

class VmpGenerator
{
public:
	err_t Generate(const vector<INode *> & nodes);
	err_t GenerateNode(INode * pNode);

	mtlid_t AddMtl(Mtl & mtl);
	int_x AddBone(INode * pBone);
protected:
	vector<MaxMtl> mtls;
	vector<vertex_t> vertices;
	vector<int_16> indices;
	vector<node_t> nodes;

	vector<INode *> bones;
};

}