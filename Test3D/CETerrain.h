#pragma once

#include "IEngine.h"
#include "CEVegetation.h"
const int_x TERRAIN_TESS_SIZE = 50;
/**
 * LOD
 * 0 1 * 1
 * 1 2 * 2
 * 2 4 * 4
 * 3 8 * 8
 * 4 16 * 16
 * 5 32 * 32
 * 6 64 * 64
 * 7 128 * 128
 * 8 256 * 256
 */
class IETerrain
{
public:
	virtual I3DScene * GetScene() const = 0;
	virtual float3 GetStep() const = 0;
	virtual sizeix GetSize() const = 0;
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

class CETerrain : public C3DNode<I3DNode>
{
public:
	CETerrain(I3DExplorer * pExplorer);
	~CETerrain();

	err_t Load(const char_16 * szTerrain);

	void Update(int_x iPeriod);
	void Render(I3DContext * pContext);
protected:
	I3DTexture * m_ptexNH;
	I3DTexture * m_ptexColor;
	I3DTexture * m_ptexLayer;
	I3DTexture * m_ptexDetail;
	I3DBuffer * m_pbufDensity;

	I3DBuffer * m_pvb;
	I3DBuffer * m_pib;

	int_x m_iWidth;
	int_x m_iHeight;
	float_32 m_fHeightScale;

	int_x m_iTriCount;
	float4x4 m_matTrans;

	vector<CEVegetation *> m_vegetations;

	static I3DBuffer * GenerateDensityFromNH(I3DTexture * ptexNH,
											 const vector<vertex_ter_t> & vertices,
											 const vector<int_16> & indices,
											 float_32 fDensityScale);
};

