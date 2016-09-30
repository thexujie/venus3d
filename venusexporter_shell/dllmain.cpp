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

// 获取导出插件的描述
VENUSEXPORTER_API const char * LibDescription()
{
	return "Venus Model Pack Exporter Shell";
}

// 获取 ClassDesc 接口的数量
VENUSEXPORTER_API int LibNumberClasses()
{
	return 1;
}

// 获取 ClassDesc 接口
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

// 获取 3ds max 版本
VENUSEXPORTER_API ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

VENUSEXPORTER_API ULONG CanAutoDefer()
{
	return 1;
}