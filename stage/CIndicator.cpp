#include "stdafx.h"
#include "CIndicator.h"
/*
* ��λ���ָʾ����
* ����ǳ������ڼ����ͷŷ�Χָʾ���������Ϸ�������������������ͶӰ��ͨ������һ�� bais�����Ƴ�ͼ�Ρ�
* ����ʱ�������෴�����̡�
* ���Ƶ� vs��hs��ds�׶ζ��Ǻ� terrain ��ͬ����ͬ���ǣ��� ps �У�Ҫ��������ԭʼ�� tex����ָʾ��ͼ���ϲ�����
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