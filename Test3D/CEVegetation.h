#pragma once

#include "BaseInc.h"

class CEVegetation : public C3DNode<I3DNode>
{
public:
	CEVegetation(I3DExplorer * pExplorer);
	~CEVegetation();

	void Transform(const float4x4 & matr);
	err_t SetMesh(const char_16 * szMesh);
	err_t SetMaterial(I3DMaterial * pMaterial);

	err_t Load(const char_16 * szMesh, const char_16 * szMaterial, const char_16 * szVegetation);
	void Update(int_x iPeriod);
	void Render(I3DContext * pContext);
private:
	I3DMesh * m_pMesh;
	I3DMaterial * m_pMaterial;
	vector<I3DMaterial *> m_materials;
	float4x4 m_transform;
	vector<float4x4> m_points;
};
