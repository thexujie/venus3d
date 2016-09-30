#pragma once

//#define _UNICODE_MODULE_FOR_MBCS_MAX
#include "max.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "3dsmaxport.h"

class VenusExporterShellClassDesc : public ClassDesc2
{
public: // ClassDesc2
	int IsPublic();
	void * Create(BOOL bLoading = FALSE);
	const TCHAR * ClassName();
	SClass_ID SuperClassID();
	Class_ID ClassID();
	const TCHAR * Category();
	const TCHAR * InternalName();
	HINSTANCE HInstance();
public: // Me
	VenusExporterShellClassDesc();
	void SetHInstance(HINSTANCE hInstance);
private:
	HINSTANCE m_hInstance;
};

class CModule
{
public:
	CModule(const TCHAR * szDll);
	~CModule();

	void Free();
	void * GetProc(const CHAR * szProc);
private:
	HMODULE m_hModule;
};
class VenusExporterShell : public SceneExport
{
public:
	int ExtCount();
	const TCHAR * Ext(int iExtIndex);
	const MCHAR * LongDesc();
	const MCHAR * ShortDesc();
	const MCHAR * AuthorName();
	const MCHAR * CopyrightMessage();
	const TCHAR * OtherMessage1();
	const TCHAR * OtherMessage2();
	unsigned int Version();
	void ShowAbout(HWND hWnd);
	int DoExport(const TCHAR * szFileName, ExpInterface * pei, Interface * pi, BOOL bSuppressPrompts, DWORD dwOptions);
	BOOL SupportsOptions(int iExtIndex, DWORD dwOptions);
};

VenusExporterShellClassDesc * GetIGameExporterDesc();
