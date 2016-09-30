#include "stdafx.h"
#include "CEVegetation.h"
#include "xml_helper.h"

CEVegetation::CEVegetation(I3DExplorer * pExplorer):
C3DNode(pExplorer), m_pMesh(nullptr), m_pMaterial(nullptr)
{
	m_points.add(float4x4_translate(float3(0, 50, 0)));
}

CEVegetation::~CEVegetation()
{
	SafeRelease(m_pMesh);
}

void CEVegetation::Transform(const float4x4 & matr)
{
	m_transform = matr;
}

err_t CEVegetation::SetMesh(const char_16 * szMesh)
{
	m_pMesh = m_pExplorer->GetMesh(szMesh);
	return err_ok;
}

err_t CEVegetation::SetMaterial(I3DMaterial * pMaterial)
{
	m_pMaterial = pMaterial;
	return err_ok;
}

err_t CEVegetation::Load(const char_16 * szMesh, const char_16 * szMaterial, const char_16 * szVegetation)
{
	if(!szMesh || !szMaterial || !szVegetation)
		return err_invalid_parameters;

	m_pMesh = m_pExplorer->GetMesh(szMesh);
	m_pMaterial = m_pExplorer->GetMaterial(szMaterial);

	object_ptr<I3DFile> pFile(m_pExplorer->GetFile(szVegetation, ResourceTypeTerrain));
	if(!pFile)
		return err_no_file;

	xml_doc terrain;
	terrain.load_text((const char_16 *)pFile->GetBuffer(), pFile->GetSize() / 2);
	m_points.resize(terrain.children.size());
	for(int_x cnt = 0; cnt < terrain.children.size(); ++cnt)
	{
		float3 point = xmlh_float3(terrain.children[cnt].text);
		m_points[cnt] = float4x4_translate(point);
	}
	return err_ok;
}

void CEVegetation::Update(int_x iPeriod)
{

}

void CEVegetation::Render(I3DContext * pContext)
{
	if(!m_pMesh)
		return;

	I3DCamera * pCamera = m_pScene->GetCamera();
	if(!pCamera)
		return;

	I3DEffect * pEffect = m_pExplorer->GetEffect(L"veg");
	if(!pEffect)
		return;

	cbuffer<36> buffer;
	buffer.offset(16);
	buffer.push(pCamera->GetMatrixVP());
	buffer.push(0.5f);
	buffer.push(1.0f);
	buffer.push(0.6f);

	pEffect->Bind(pContext);

	pEffect->SetVertexFormat(VertexFormatPTN);

	int_x iCount = m_pMesh->GetNodeCount();
	m_pMesh->RenderBeg(pContext);
	m_pMaterial->RenderBeg(pContext);
	for(int_x iVeg = 0; iVeg < m_points.size(); ++iVeg)
	{
		buffer.set(0, m_points[iVeg]);
		pEffect->SetBuffer(ShaderTypeVertex, 0, 0, buffer.size(), buffer);

		for(int_x iNode = 0; iNode < iCount; ++iNode)
		{
			m_pMaterial->Render(pContext, iNode);
			m_pMesh->Render(pContext, iNode);
		}
	}
	m_pMaterial->RenderEnd(pContext);
	m_pMesh->RenderEnd(pContext);

	pEffect->Bind(nullptr);
}
