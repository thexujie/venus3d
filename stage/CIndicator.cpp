#include "stdafx.h"
#include "CIndicator.h"
/*
* 如何绘制指示器？
* 这个非常类似于技能释放范围指示器，从正上方（或者其他方向）向下投影，通过增加一个 bais，绘制出图形。
* 绘制时，采用相反的流程。
* 绘制的 vs、hs、ds阶段都是和 terrain 相同，不同的是，在 ps 中，要反向计算出原始的 tex，在指示器图形上采样。
*/
CIndicator::CIndicator(I3DExplorer * pExplorer):C3DNode(pExplorer)
{

}

CIndicator::~CIndicator()
{

}

err_t CIndicator::Create()
{
	vertex_pnt vtxs[4] = {};
	vtxs[0].position = float3(-50, 50, -50);
	vtxs[1].position = float3(50, 50, -50);
	vtxs[2].position = float3(50, 50, 50);
	vtxs[3].position = float3(-50, 50, 50);
	m_pvb = GetVedio()->CreateBuffer();
	m_pvb->Create(sizeof(vertex_pnt), 4, bufferusage_dynamic, bufferbind_vertex, bufferaccess_write, cmode_invalid, vtxs, sizeof(vtxs), 0);

	m_pEffect = m_pExplorer->GetEffect(L"indicator");
	SafeAddRef(m_pEffect);
	m_pMaterial = m_pExplorer->GetMaterial(L"indicator.mtl");
	return err_ok;
}


//void CIndicator::RenderGenerate(vector<RenderSourceT> & sources, vector<RenderCommandT> & commands) const
//{
//	RenderSourceT source = {this, m_pEffect, m_pMaterial, m_pvb, nullptr,
//		VertexFormatPTN, indexformat_unkown, primitiveformat_tri_strip, 1, float4x4()};
//	RenderCommandT command = {0, 0, 4, 0, 0, 0};
//	sources.add(source);
//	commands.add(command);
//}

void CIndicator::Render(I3DContext * pContext, int_x iSubSet) const
{

}