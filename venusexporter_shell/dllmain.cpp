#include "stdafx.h"
#include "venusexporter_shell.h"

BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		GetIGameExporterDesc()->SetHInstance(hModule);
		DisableThreadLibraryCalls(hModule);
	}

	return (TRUE);
}

// ��ȡ�������������
VENUSEXPORTER_API const char * LibDescription()
{
	return "Venus Model Pack Exporter Shell";
}

// ��ȡ ClassDesc �ӿڵ�����
VENUSEXPORTER_API int LibNumberClasses()
{
	return 1;
}

// ��ȡ ClassDesc �ӿ�
VENUSEXPORTER_API ClassDesc * LibClassDesc(int iIndex)
{
	switch(iIndex)
	{
	case 0: 
		return GetIGameExporterDesc();
	default: 
		return nullptr;
	}
}

// ��ȡ 3ds max �汾
VENUSEXPORTER_API ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

VENUSEXPORTER_API ULONG CanAutoDefer()
{
	return 1;
}