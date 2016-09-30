#include "stdafx.h"
#include "CVmpWindow.h"

CVmpWindow::CVmpWindow()
{
	SetRect(0, 0, 600, 400);
	m_tab.SetRect(5, 5, 590, 390);
	m_staVertextFormat.SetRect(10, 40, 60, 20);
	m_gpbOptions.SetRect(10, 25, 160, 200);
	m_gbVertexFormat.SetRect(180, 25, 160, 200);
	m_btnOk.SetRect(400, 360, 80, 25);
	m_btnCancel.SetRect(490, 360, 80, 25);

	SetText(L"Venus Exporter");

	m_btnOk.SetText(L"ȷ��(&O)");
	m_btnCancel.SetText(L"ȡ��(&C)");
	AddControl(&m_tab);
	AddControl(&m_btnOk);
	AddControl(&m_btnCancel);
	m_tab.AddTab(L"��������");
	m_tab.AddTab(L"�߼�ѡ��");

	m_gpbOptions.SetText(L"��������");

	m_chbExpMesh.SetText(L"����");
	m_chbExpAnim.SetText(L"����");
	m_chbLocalCoord.SetText(L"ʹ�þֲ�����ϵ");

	m_chbExpMesh.SetRect(5, 5, 148, 20);
	m_chbExpAnim.SetRect(5, 30, 148, 20);
	m_chbLocalCoord.SetRect(5, 150, 148, 20);

	m_gpbOptions.AddControl(&m_chbExpMesh);
	m_gpbOptions.AddControl(&m_chbExpAnim);
	m_gpbOptions.AddControl(&m_chbLocalCoord);
	m_tab.AddTabControl(0, &m_gpbOptions);

	m_gbVertexFormat.SetText(L"�����ʽ");

	m_chbVertPosition.SetText(L"��������");
	m_chbVertTexcoord.SetText(L"��������");

	m_chbVertWeight.SetText(L"Ȩ��");
	m_chbVertNormal.SetText(L"����");
	m_chbVertColor.SetText(L"������ɫ");

	m_chbVertPosition.SetRect(5, 5, 90, 20);
	m_chbVertTexcoord.SetRect(5, 30, 90, 20);
	m_chbVertWeight.SetRect(5, 55, 90, 20);
	m_chbVertNormal.SetRect(5, 80, 90, 20);
	m_chbVertColor.SetRect(5, 405, 90, 20);

	m_chbVertPosition.SetEnable(false);

	m_gbVertexFormat.AddControl(&m_chbVertPosition);
	m_gbVertexFormat.AddControl(&m_chbVertTexcoord);
	m_gbVertexFormat.AddControl(&m_chbVertWeight);
	m_gbVertexFormat.AddControl(&m_chbVertNormal);
	m_gbVertexFormat.AddControl(&m_chbVertColor);
	m_tab.AddTabControl(0, &m_gbVertexFormat);

	m_btnOk.SetId(IdOk);
	m_btnCancel.SetId(IdCancel);
	m_btnOk.MouseClickL += bind(this, &CVmpWindow::OnClick);
	m_btnCancel.MouseClickL += bind(this, &CVmpWindow::OnClick);

	m_chbExpMesh.SetChecked(true);
	m_chbVertPosition.SetChecked(true);
	m_chbVertTexcoord.SetChecked(true);
}

CVmpWindow::~CVmpWindow()
{
}

void CVmpWindow::OnClose()
{
	Win32::Quit(0);
}

int_x CVmpWindow::OnClick(IControl * pControl, pointix point)
{
	switch(pControl->GetId())
	{
	case IdOk:
		EndRun(DialogResultOk);
		break;
	case IdCancel:
		EndRun(DialogResultCancel);
		break;
	default:
		break;
	}
	return 0;
}

void CVmpWindow::SetMode(ExportModeE eMode)
{
	if(eMode == ExportModeMesh)
	{
		m_chbExpMesh.SetChecked(true);
		m_chbExpAnim.SetChecked(false);
	}
	else if(eMode == ExportModeAnim)
	{
		m_chbExpMesh.SetChecked(false);
		m_chbExpAnim.SetChecked(true);
	}
	else
	{
		m_chbExpMesh.SetChecked(false);
		m_chbExpAnim.SetChecked(false);
	}
}

ExportModeE CVmpWindow::GetMode() const
{
	if(m_chbExpMesh.IsChecked())
		return ExportModeMesh;
	else
		return ExportModeAnim;
}

vertexformat_e CVmpWindow::GetVertexFormat() const
{
	int_x iVertexFormat = vertexformat_pos;

	if(m_chbVertNormal.IsChecked())
		iVertexFormat |= vertexformat_nor;

	if(m_chbVertTexcoord.IsChecked())
		iVertexFormat |= vertexformat_tex;

	if(m_chbVertWeight.IsChecked())
		iVertexFormat |= vertexformat_wgt;

	return (vertexformat_e)iVertexFormat;
}

bool CVmpWindow::IsUseLocalCoord() const
{
	return m_chbLocalCoord.IsChecked();
}
