#include "stdafx.h"
#include "CFileDialog.h"

VENUS_BEG

CFileDialog::CFileDialog()
{
	SetFormType(FormTypePopup);
	SetBorderType(BorderType3Inactive);
	SetCaptionHeight(-1);
	SetResizeBorder(edgeix(4));

	SetBoxes(true, true, true);
	SetRect(0, 0, 600, 400);
	m_txlPath.SetRect(6, 6, 584, 24);
	m_ftv.SetRect(6, 34, 200, 260);
	m_flv.SetRect(210, 34, 380, 260);
	m_staName.SetRect(3, 307, 80, 24);
	m_txlName.SetRect(90, 307, 500, 24);
	m_staType.SetRect(3, 337, 80, 24);
	m_txlType.SetRect(90, 337, 500, 24);

	m_staName.SetText(L"文件名(&N)");
	m_staName.SetTextAlign(AlignRightCenterY);
	m_staType.SetText(L"文件类型(&T)");
	m_staType.SetTextAlign(AlignRightCenterY);

	AddControl(&m_txlPath);
	AddControl(&m_ftv);
	AddControl(&m_flv);
	AddControl(&m_staName);
	AddControl(&m_txlName);
	AddControl(&m_staType);
	AddControl(&m_txlType);

	m_txlPath.SetAnchor(AlignLTR);
	m_ftv.SetAnchor(AlignLTB);
	m_flv.SetAnchor(AlignLTRB);
	m_staName.SetAnchor(AlignBL);
	m_txlName.SetAnchor(AlignLRB);
	m_staType.SetAnchor(AlignBL);
	m_txlType.SetAnchor(AlignLRB);

	m_txlType.AddItem(L"文本文件(.txt)");
	m_txlType.AddItem(L"图像文件(.jpg;.png;.gif)");

	m_ftv.SetScrollBarX(nullptr);

	m_ftv.SetCanSelectNull(false);

	m_ftv.SelectedItemChanged.connect(this, &CFileDialog::OnFolderSelected);
	m_flv.MouseDBClickL.connect(this, &CFileDialog::OnListDBClick);

}

CFileDialog::~CFileDialog()
{

}

void CFileDialog::SetRoot(const char_16 * szRoot)
{
	m_ftv.SetRoot(szRoot);
}

int_x CFileDialog::OnFolderSelected(IControl * pControl, TreeItemT * pItem)
{
	textw path = CFolderTreeView::GetItemPath(pItem);
	m_flv.SetPath(path);
	m_txlPath.SetText(path);
	return 0;
}

int_x CFileDialog::OnListDBClick(IControl * pControl, pointix point)
{
	FolderListViewRowT * pRow = (FolderListViewRowT *)m_flv.GetRowSelected();
	if(!pRow)
		return 0;

	pathinfo_t pinfo(pRow->path);
	if(pinfo.exists)
		m_ftv.SelectPath(pinfo.path);
	return 0;
}

VENUS_END
