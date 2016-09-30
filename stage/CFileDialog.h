#pragma once

#include "BaseInc.h"

VENUS_BEG

class CFileDialog : public CForm
{
public:
	CFileDialog();
	~CFileDialog();

	void SetRoot(const char_16 * szRoot);

	int_x OnFolderSelected(IControl * pControl, TreeItemT * pItem);
	int_x OnListDBClick(IControl * pControl, pointix point);
protected:
	CTextLine m_txlPath;
	CFolderTreeView m_ftv;
	CFolderListView m_flv;

	CStatic m_staName;
	CTextLine m_txlName;
	CStatic m_staType;
	CComboBox m_txlType;
};

VENUS_END
