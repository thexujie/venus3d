#pragma once

#include "BaseInc.h"

class CVmpWindow : public CForm
{
public:
	CVmpWindow();
	~CVmpWindow();

	void OnClose();

	int_x OnClick(IControl * pControl, pointix point);

	void SetMode(ExportModeE eMode);
	ExportModeE GetMode() const;
	vertexformat_e GetVertexFormat() const;
	bool IsUseLocalCoord() const;
private:
	CTabControl m_tab;

	CStatic m_staVertextFormat;

	CGroupBox m_gpbOptions;
	CGroupBox m_gbVertexFormat;

	CCheckBox m_chbExpMesh;
	CCheckBox m_chbExpAnim;
	CCheckBox m_chbLocalCoord;

	CCheckBox m_chbVertPosition;
	CCheckBox m_chbVertTexcoord;
	CCheckBox m_chbVertWeight;
	CCheckBox m_chbVertNormal;
	CCheckBox m_chbVertColor;

	CButton m_btnOk;
	CButton m_btnCancel;

	enum Id
	{
		IdOk,
		IdCancel,
	};
};
