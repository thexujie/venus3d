#include "stdafx.h"
#include "CEditorApp.h"

CEditorApp::CEditorApp()
{
}

CEditorApp::~CEditorApp()
{
}

void CEditorApp::AddWindow(IEditorWindow * pWindow)
{
	m_windows.add(pWindow);
}

int_x CEditorApp::OnRun()
{
	for(int_x cnt = 0; cnt < m_windows.size(); ++cnt)
		m_windows[cnt]->OnRun();
	return err_ok;
}
