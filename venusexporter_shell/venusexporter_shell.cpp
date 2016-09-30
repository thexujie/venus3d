#include "stdafx.h"
#include "venusexporter_shell.h"
#include "Text.h"

#define Class_ID_GameExporterShell Class_ID(0x6bb37072, 0xc617eff)

static VenusExporterShellClassDesc g_VenusExporterShellClassDesc;
VenusExporterShellClassDesc* GetIGameExporterDesc() 
{
	return &g_VenusExporterShellClassDesc; 
}


int VenusExporterShellClassDesc::IsPublic()
{
	return TRUE;
}

void * VenusExporterShellClassDesc::Create(BOOL bLoading)
{
	return new VenusExporterShell();
}

const TCHAR * VenusExporterShellClassDesc::ClassName()
{
	return _T("VenusExporterShell");
}

SClass_ID VenusExporterShellClassDesc::SuperClassID()
{
	return SCENE_EXPORT_CLASS_ID;
}

Class_ID VenusExporterShellClassDesc::ClassID()
{
	return Class_ID_GameExporterShell;
}

const TCHAR * VenusExporterShellClassDesc::Category()
{
	return _M("Venus");
}

const TCHAR * VenusExporterShellClassDesc::InternalName()
{
	return _M("VenusExporterShell");
}

HINSTANCE VenusExporterShellClassDesc::HInstance()
{
	return m_hInstance;
}

VenusExporterShellClassDesc::VenusExporterShellClassDesc():
m_hInstance(NULL)
{

}

void VenusExporterShellClassDesc::SetHInstance(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
}




CModule::CModule(const TCHAR * szDll)
{
	m_hModule = LoadLibraryExW(szDll, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
}

void CModule::Free()
{
	if(m_hModule)
	{
		FreeLibrary(m_hModule);
		m_hModule = NULL;
	}
}

CModule::~CModule()
{
	if(m_hModule)
	{
		FreeLibrary(m_hModule);
		m_hModule = NULL;
	}
}

void * CModule::GetProc(const CHAR * szProc)
{
	if(m_hModule)
		return GetProcAddress(m_hModule, szProc);
	else
		return nullptr;
}


/**
* .vmesh Íø¸ñ
* .vact ¶¯»­
*/
int VenusExporterShell::ExtCount()
{
	return 2;
}

const TCHAR * VenusExporterShell::Ext(int iExtIndex)
{
	switch(iExtIndex)
	{
	case 0:
		return _T("xemesh");
	case 1:
		return _T("xebone");
	default:
		return nullptr;
	}
}

const MCHAR * VenusExporterShell::LongDesc()
{
	return _M("Venus Model Exporter.");
}

const MCHAR * VenusExporterShell::ShortDesc()
{
	return _M("Venus Model Exporter");
}

const MCHAR * VenusExporterShell::AuthorName()
{
	return _M("xujie");
}
const MCHAR * VenusExporterShell::CopyrightMessage()
{
	return _M("");
}
const TCHAR * VenusExporterShell::OtherMessage1()
{
	return _M("");
}
const TCHAR * VenusExporterShell::OtherMessage2()
{
	return _M("");
}

unsigned int VenusExporterShell::Version()
{
	return 100;
}
void VenusExporterShell::ShowAbout(HWND hWnd)
{
	MessageBox(hWnd, _T("Venus Model Exporter\nAll rights reserved."), _T("Venus Exporter"), MB_OK);
}


typedef int(*DoExportFunT)(const TCHAR * szFileName, ExpInterface * pei, Interface * pi, BOOL bSuppressPrompts, DWORD dwOptions);
int DoExportNow(DoExportFunT pfnDoExport, const TCHAR * szFileName,
				ExpInterface * pei, Interface * pi, BOOL bSuppressPrompts, DWORD dwOptions)
{
	int iRet = IMPEXP_SUCCESS;
	__try
	{
		iRet = pfnDoExport(szFileName, pei, pi, bSuppressPrompts, dwOptions);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		iRet = IMPEXP_FAIL;
	}
	return iRet;
}

int VenusExporterShell::DoExport(const TCHAR * szFileName, ExpInterface * pei, Interface * pi, BOOL bSuppressPrompts, DWORD dwOptions)
{
	HMODULE hModule = GetModuleHandle(_T("venusexporter_shell.dle"));
	if(!hModule)
		return IMPEXP_FAIL;

	TCHAR szPluginDir[MAX_PATH] = {};
	TCHAR szModuleFile[MAX_PATH] = {};
	TCHAR szDllFile[MAX_PATH] = {0};

	GetModuleFileName(hModule, szModuleFile, MAX_PATH);
	int_x iLen = textsplitpath_folder(szModuleFile, -1, szPluginDir, MAX_PATH);

	textcpy(szDllFile, MAX_PATH, szPluginDir, iLen);
	textcat(szDllFile, MAX_PATH, _T("venusexporter.dll"), -1);

	CModule module(szDllFile);

	DoExportFunT pfnDoExport = (DoExportFunT)module.GetProc("DoExport");
	if(!pfnDoExport)
		return IMPEXP_FAIL;
	else
		return DoExportNow(pfnDoExport, szFileName, pei, pi, bSuppressPrompts, dwOptions);
}

BOOL VenusExporterShell::SupportsOptions(int iExtIndex, DWORD dwOptions)
{
	return TRUE;
}
