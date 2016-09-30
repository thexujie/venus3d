#include "stdafx.h"
#include "CETerrain.h"

CETerrain::CETerrain(I3DExplorer * pExplorer) :
C3DNode(pExplorer),
m_ptexNH(nullptr), m_ptexColor(nullptr), m_ptexLayer(nullptr), m_ptexDetail(nullptr), m_pbufDensity(nullptr),
m_pvb(nullptr), m_pib(nullptr), m_fHeightScale(1.0f)
{
	m_matTrans.identity();
}

CETerrain::~CETerrain()
{
	SafeRelease(m_ptexNH);
	SafeRelease(m_ptexColor);
	SafeRelease(m_ptexLayer);
	SafeRelease(m_ptexDetail);
	SafeRelease(m_pvb);
	SafeRelease(m_pib);
}

err_t CETerrain::Load(const char_16 * szTerrain)
{
	I3DExplorer * pExplorer = m_pExplorer;
	if(!pExplorer)
		return err_not_initialized;

	xml_doc terrain;
	object_ptr<I3DFile> pFile(pExplorer->GetFile(szTerrain, ResourceTypeTerrain));
	if(!pFile)
		return err_no_file;

	terrain.load_text((const char_16 *)pFile->GetBuffer(), pFile->GetSize() / 2);

	float_32 step_x = terrain[L"step_x"].value.tof32(1.0f);
	float_32 step_z = terrain[L"step_z"].value.tof32(1.0f);
	m_fHeightScale = terrain[L"step_y"].value.tof32(1.0f);

	int_x iWidth = TERRAIN_TESS_SIZE;
	int_x iHeight = TERRAIN_TESS_SIZE;

	int_x iVetCount = TERRAIN_TESS_SIZE * TERRAIN_TESS_SIZE;
	int_x iVetSize = iVetCount * sizeof(vertex_ter_t);
	vector<vertex_ter_t> vertices(iVetCount, iVetCount);
	float_32 fx_beg = -0.5f * iWidth * step_x;
	float_32 fz_beg = 0.5f * iHeight * step_z;
	float_32 fx = fx_beg, fz = fz_beg;
	for(int_x row = 0; row < iHeight; ++row)
	{
		for(int_x col = 0; col < iWidth; ++col)
		{
			vertex_ter_t & vertex = vertices[row * iWidth + col];
			vertex.pos = float3(fx, 0.0f, fz);
			vertex.tex = float2((float_32)col / iWidth, (float_32)row / iHeight);
			vertex.nom.set(0.0f, 1.0f, 0.0f);
			vertex.bin.set(0.0f, 0.0f, -1.0f);
			vertex.tan.set(1.0f, 0.0f, 0.0f);

			fx += step_x;
		}
		fz -= step_z;
		fx = fx_beg;
	}
	m_pvb = GetVedio()->CreateBuffer();
	m_pvb->Create(sizeof(vertex_ter_t), iVetSize, BufferUsageDefault, BufferBindVertex, BufferAccessNone, vertices.buffer(), iVetSize);

	m_iTriCount = (iWidth - 1) * (iHeight - 1) * 2;
	int_x iIdxCount = (iWidth - 1) * (iHeight - 1) * 2 * 3;
	int_x iIdxSize = iIdxCount * sizeof(int_16);
	vector<int_16> indices(iIdxCount, iIdxCount);
	for(int_x index = 0, row = 0; row < iHeight - 1; ++row)
	{
		for(int_x col = 0; col < iWidth - 1; ++col)
		{
			int_x index0 = row * iWidth + col;
			int_x index1 = index0 + 1;
			int_x index2 = index1 + iWidth;
			int_x index3 = index0 + iWidth;
			indices[index++] = (int_32)index0;
			indices[index++] = (int_32)index1;
			indices[index++] = (int_32)index2;
			indices[index++] = (int_32)index2;
			indices[index++] = (int_32)index3;
			indices[index++] = (int_32)index0;
		}
	}

	m_pib = GetVedio()->CreateBuffer();
	m_pib->Create(sizeof(int_32), iIdxSize, BufferUsageDefault, BufferBindIndex, BufferAccessNone, indices, iIdxSize);

	m_ptexNH = pExplorer->GetTexture(terrain(L"nheight").text, ResourceTypeTerrain);
	m_ptexColor = pExplorer->GetTexture(terrain(L"color").text, ResourceTypeTerrain);
	m_ptexLayer = pExplorer->GetTexture(terrain(L"layer").text, ResourceTypeTerrain);
	m_ptexDetail = pExplorer->GetTexture(terrain(L"detail").text, ResourceTypeTerrain);
	m_pbufDensity = GenerateDensityFromNH(m_ptexNH, vertices, indices, step_x);

	// 植被信息
	const xml_node & node_veg = terrain(L"vegetations");
	if(node_veg.is_valid())
	{
		for(int_x cnt = 0; cnt < node_veg.children.size(); ++cnt)
		{
			const xml_node & child = node_veg.children[cnt];
			CEVegetation * pVegetation = new CEVegetation(m_pExplorer);
		}
	}

	return err_ok;
}

void CETerrain::Update(int_x iPeriod)
{
	I3DCamera * pCamera = m_pScene->GetCamera();
	if(!pCamera)
		return;
}

void CETerrain::Render(I3DContext * pContext)
{
	if(!m_pib || !m_pvb)
		return;

	I3DCamera * pCamera = m_pScene->GetCamera();
	if(!pCamera)
		return;

	const char_16 * szEffect = L"terrain.eff";
	if(m_eFillMode == FillModeWireframe)
		szEffect = L"terrain_frame.eff";

	I3DEffect * pEffect = m_pExplorer->GetEffect(szEffect);
	if(!pEffect)
		return;

	pEffect->Bind(pContext);

	float4x4 matrVP = pCamera->GetMatrixV() * pCamera->GetMatrixP();
	view_frustum_t vf(matrVP);

	cbuffer<56> buffer;
	buffer.push(m_matTrans);
	buffer.push(matrVP);
	buffer.push(vf.planes[0]);
	buffer.push(vf.planes[1]);
	buffer.push(vf.planes[2]);
	buffer.push(vf.planes[3]);
	//buffer.push(float4x4());
	buffer.push(pCamera->GetPosition());
	buffer.push(0.0f);
	buffer.push(pCamera->GetVisual());
	buffer.push(20.0f);
	buffer.push(m_fHeightScale);
	buffer.push(50.0f);

	pEffect->SetVertexFormat(VertexFormatPTN);
	pEffect->SetBuffer(ShaderTypeVertex, 0, 0, buffer.size(), buffer);
	pEffect->SetBuffer(ShaderTypeHull, 0, 0, buffer.size(), buffer);
	pEffect->SetBuffer(ShaderTypeDomain, 0, 0, buffer.size(), buffer);
	pEffect->SetBuffer(ShaderTypePixel, 0, 0, buffer.size(), buffer);

	m_pbufDensity->Bind(pContext, ShaderTypeHull, 1, cmode_b32g32r32f);

	pContext->SetTexture(ShaderTypeHull, 0, m_ptexNH);
	pContext->SetTexture(ShaderTypeDomain, 0, m_ptexNH);
	pContext->SetTexture(ShaderTypePixel, 0, m_ptexColor);
	pContext->SetTexture(ShaderTypePixel, 1, m_ptexLayer);
	pContext->SetTexture(ShaderTypePixel, 2, m_ptexDetail);

	pContext->SetVertexBuffer(m_pvb, VertexFormatPosTexNorBinTan);
	pContext->SetIndexBuffer(m_pib, IndexFormat16);
	pContext->DrawIndexed(0, 0, m_iTriCount * 3, PrimitiveFormatTriPatchList);

	pEffect->Bind(nullptr);
}

static float GetMaximumDensityAlongEdge(uint_8 * pDensities, int_x dwWidth, int_x dwHeight,
								 float2 fTexcoord0, float2 fTexcoord1)
{
	// Convert texture coordinates to texel coordinates using round-to-nearest
	int_x nU0 = round(fTexcoord0.x * (dwWidth - 1));
	int_x nV0 = round(fTexcoord0.y * (dwHeight - 1));
	int_x nU1 = round(fTexcoord1.x * (dwWidth - 1));
	int_x nV1 = round(fTexcoord1.y * (dwHeight - 1));

	// Wrap texture coordinates
	nU0 = nU0 % dwWidth;
	nV0 = nV0 % dwHeight;
	nU1 = nU1 % dwWidth;
	nV1 = nV1 % dwHeight;

	// Find how many texels on edge
	int_x nNumTexelsOnEdge = maxof(abs(nU1 - nU0), abs(nV1 - nV0)) + 1;

	float fU, fV;
	float fMaxDensity = 0.0f;
	for(int_x i = 0; i < nNumTexelsOnEdge; ++i)
	{
		// Calculate current texel coordinates
		fU = lerp((float)nU0, (float)nU1, ((float)i) / (nNumTexelsOnEdge - 1));
		fV = lerp((float)nV0, (float)nV1, ((float)i) / (nNumTexelsOnEdge - 1));

		// Round texel texture coordinates to nearest
		int_x nCurrentU = round(fU);
		int_x nCurrentV = round(fV);

		// Update max density along edge
		uint_8 density = *(pDensities + nCurrentV * dwWidth + nCurrentU);
		fMaxDensity = maxof(fMaxDensity, (float)density / 255.0f);
	}

	return fMaxDensity;
}

I3DBuffer * CETerrain::GenerateDensityFromNH(I3DTexture * ptexNH,
										 const vector<vertex_ter_t> & vertices,
										 const vector<int_16> & indices,
										 float_32 fDensityScale)
{
	if(!ptexNH)
		return nullptr;

	TextureDescT desc = ptexNH->GetDesc();
	if(desc.CMode != cmode_a8r8g8b8)
		return nullptr;

	int_x iTriCount = indices.size() / 3;
	float_32 fDiff = 1.0f / 255.0f;
	int_x iSize = desc.Width * desc.Height;
	uint_8 * pDensity = new uint_8[iSize];

	I3DVedio * pVedio = ptexNH->GetVedio();
	I3DTexture * pStaging = pVedio->CreateTexture();
	pStaging->Create(desc.Width, desc.Height, desc.CMode, 1, BufferUsageStaging, BufferBindNone, BufferAccessRead);
	ptexNH->Copy(pStaging);

	MappedBufferT buffer = {};
	pStaging->Map(buffer, BufferMapRead);

	const color_argb_t * pixels = (const color_argb_t *)buffer.Data;

	for(int_x row = 0, index = 0; row < desc.Height; ++row)
	{
		for(int_x col = 0; col < desc.Width; ++col, ++index)
		{
			uint_8 uiDensity = 1;

			// 边框的密度为 1
			if(row && col &&
			   (row != desc.Height - 1) &&
			   (col != desc.Width - 1))
			{
				int_x index = row * desc.Width + col;
				float_32 heights[3][3];
				heights[0][0] = pixels[index - desc.Width - 1].a / 255.0f;
				heights[0][1] = pixels[index - desc.Width].a / 255.0f;
				heights[0][2] = pixels[index - desc.Width + 1].a / 255.0f;

				heights[1][0] = pixels[index - 1].a / 255.0f;
				heights[1][1] = pixels[index].a / 255.0f;
				heights[1][2] = pixels[index + 1].a / 255.0f;

				heights[2][0] = pixels[index + desc.Width - 1].a / 255.0f;
				heights[2][1] = pixels[index + desc.Width].a / 255.0f;
				heights[2][2] = pixels[index + desc.Width + 1].a / 255.0f;

				float_32 density = 0.0f;
				float_32 densities[4];
				densities[0] = abs((heights[1][2] - heights[1][1]) - (heights[1][1] - heights[1][0]));
				densities[1] = abs((heights[2][1] - heights[1][1]) - (heights[1][1] - heights[0][1]));
				densities[2] = abs((heights[2][0] - heights[1][1]) - (heights[1][1] - heights[0][2]));
				densities[3] = abs((heights[2][2] - heights[1][1]) - (heights[1][1] - heights[0][0]));

				// 线性分配
				float_32 rates[4] = {0.293f, 0.293f, 0.207f, 0.207f};
				for(int_x cnt = 0; cnt < 4; ++cnt)
				{
					if(densities[cnt] >= fDiff)
						density += rates[cnt] * densities[cnt];
				}

				density *= fDensityScale;
				density = maxof(minof(density, 1.0f), 1.0f / 255.0f);
				uiDensity = (uint_8)(density * 255.0f);
			}
			pDensity[index] = uiDensity;
		}
	}

	pStaging->Unmap();
	SafeRelease(pStaging);

	float GetMaximumDensityAlongEdge(uint_8 * pDensities, int_x dwWidth, int_x dwHeight,
									 float2 fTexcoord0, float2 fTexcoord1);
	vector<float3> edge_densities(iTriCount, iTriCount);
	// 计算每个 patch 的密度
	for(int tri = 0; tri < iTriCount; ++tri)
	{
		const vertex_ter_t & vertex0 = vertices[indices[tri * 3 + 0]];
		const vertex_ter_t & vertex1 = vertices[indices[tri * 3 + 1]];
		const vertex_ter_t & vertex2 = vertices[indices[tri * 3 + 2]];

		float_32 fEdgeDensity0 = GetMaximumDensityAlongEdge(pDensity, desc.Width, desc.Height, vertex0.tex, vertex1.tex);
		float_32 fEdgeDensity1 = GetMaximumDensityAlongEdge(pDensity, desc.Width, desc.Height, vertex1.tex, vertex2.tex);
		float_32 fEdgeDensity2 = GetMaximumDensityAlongEdge(pDensity, desc.Width, desc.Height, vertex2.tex, vertex0.tex);

		// Store edge density in x,y,z and store maximum density in .w
		edge_densities[tri] = float3(fEdgeDensity1, fEdgeDensity2, fEdgeDensity0);
	}
	delete pDensity;

	I3DBuffer * pbufDensity = pVedio->CreateBuffer();
	pbufDensity->Create(sizeof(float3), iTriCount * sizeof(float3),
						  BufferUsageDefault, BufferBindResources, BufferAccessNone,
						  (const void *)edge_densities, iTriCount * sizeof(float3), 0);
	return pbufDensity;
}
