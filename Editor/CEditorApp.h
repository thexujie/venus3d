#pragma once

#include "BaseInc.h"

class VENUS_API CEditorApp : public CWin32App
{
public:
	CEditorApp();
	~CEditorApp();

	void AddWindow(IEditorWindow * pWindow);
	int_x OnRun();


protected:
	vector<IEditorWindow *> m_windows;
};

