#include "stdafx.h"
#include "callstack.h"

#include <WinDNS.h>
#include <DbgHelp.h>
#include <Psapi.h>
#pragma comment( lib, "Dbghelp.lib" )
#pragma comment( lib, "Psapi.lib" )

static BOOL m_bInitialized;
const char_8 NONE_SYMBOL[] = "[框架可能不正确和/或缺失]";

static void GenerateSEH(callstack_t * pStack, EXCEPTION_POINTERS * pExp, int_x iSkip)
{
	if(pStack)
		pStack->generate(pExp->ContextRecord, iSkip);
}

bool symblehlp_initialize()
{
	if(m_bInitialized)
	{
		return true;
	}
	else
	{
		HANDLE hProcess = GetCurrentProcess();
		DWORD dwSymOpts = SymGetOptions();
		dwSymOpts |= (SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
		SymSetOptions(dwSymOpts);

		// 初始化符号
		if(!SymInitialize(hProcess, nullptr, TRUE))
			return false;

		// 加载模块
		const int_x MAX_HANDLES = 4096;
		HMODULE arrModules[MAX_HANDLES] = {0};
		DWORD dwBytes = 0;
		if(!EnumProcessModules(hProcess, arrModules, sizeof(arrModules), &dwBytes))
			return false;

		// 尝试为每一个模块加载符号
		char_8 szModuleName[4096] = {0};
		char_8 szImageName[4096] = {0};
		MODULEINFO mInfo;

		int_x iCount = dwBytes / sizeof(HMODULE);
		DWORD64 dwLoadState = 0;
		for(int_x cnt = 0; cnt < iCount; ++cnt)
		{
			HMODULE hModule = arrModules[cnt];

			GetModuleInformation(hProcess, hModule, &mInfo, sizeof(mInfo));
			GetModuleFileNameExA(hProcess, hModule, szImageName, 4096);
			GetModuleBaseNameA(hProcess, hModule, szModuleName, 4096);
			dwLoadState = SymLoadModule64(hProcess, hModule, szImageName, szModuleName, (DWORD64)mInfo.lpBaseOfDll, mInfo.SizeOfImage);
		}
		m_bInitialized = TRUE;
		return true;
	}
}

bool callstack_t::generate(const void * pContext, int_x iSkip)
{
	if(!pContext)
	{
		int* pErrorPointer = NULL;
		__try
		{
			*pErrorPointer = 0;
		}
		__except(GenerateSEH(this, GetExceptionInformation(), 1), EXCEPTION_EXECUTE_HANDLER)
		{

		}
		return true;
	}

	frames.clear();
	HANDLE hProcess = ::GetCurrentProcess();
	HANDLE hThread = ::GetCurrentThread();

	// 分析调用堆栈
	int_x iDepth = 0;
	int_64 arrAddr[STACK_MAX_FRAME];
#ifdef _WIN64
	DWORD dwPlatform = IMAGE_FILE_MACHINE_AMD64;
#else
	DWORD dwPlatform = IMAGE_FILE_MACHINE_I386;
#endif
	void * pRecord = (void *)pContext;
	STACKFRAME64 sFrame = {};
	sFrame.AddrPC.Mode = AddrModeFlat;
	sFrame.AddrStack.Mode = AddrModeFlat;
	sFrame.AddrFrame.Mode = AddrModeFlat;

	while(StackWalk64(dwPlatform, hProcess, hThread, &sFrame, pRecord, nullptr,
		SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
	{
		if(!sFrame.AddrFrame.Offset)
			break;

		arrAddr[iDepth++] = (int_64)sFrame.AddrPC.Offset;
		if(iDepth >= STACK_MAX_FRAME)
			break;
	}

	// 获取每帧的 模块名、文件名、行号
	for(int_x cnt = iSkip; cnt < iDepth; ++cnt)
	{
		int_64 iAddr = arrAddr[cnt];
		stackframe_t & frame = frames.add();
		frame.addr = iAddr;

		// 函数名
		DWORD64 dwDisplacementFun = 0;
		char_8 szSymbol[sizeof(IMAGEHLP_SYMBOL64) + STACK_MAX_FUNCTION] = {0};
		IMAGEHLP_SYMBOL64 * pSymbol = (IMAGEHLP_SYMBOL64 *)szSymbol;
		pSymbol->SizeOfStruct = sizeof(szSymbol);
		pSymbol->MaxNameLength = STACK_MAX_FUNCTION;
		if(::SymGetSymFromAddr64(hProcess, iAddr, &dwDisplacementFun, pSymbol))
		{
			int_x index = 0;
			while(pSymbol->Name[index] < 32 || pSymbol->Name[index] > 127)
				++index;

			textcpy(frame.function, STACK_MAX_FUNCTION, pSymbol->Name + index, -1);
		}
		else
		{
			textcpy(frame.function, STACK_MAX_FUNCTION, NONE_SYMBOL, -1);
		}

		// 源文件路径和行号
		DWORD dwDisplacementLine = 0;
		IMAGEHLP_LINE64 imgLine = {sizeof(imgLine)};
		if(::SymGetLineFromAddr64(hProcess, iAddr, &dwDisplacementLine, &imgLine))
		{
			textcpy(frame.file, STACK_MAX_FILE, imgLine.FileName, -1);
			frame.line = (int_x)imgLine.LineNumber;
		}

		// 模块路径	
		IMAGEHLP_MODULE64 imgModule = {sizeof(imgModule)};
		if(::SymGetModuleInfo64(hProcess, iAddr, &imgModule))
		{
			textcpy(frame.module, STACK_MAX_FILE, imgModule.ImageName, -1);
		}
	}
	return true;
}
