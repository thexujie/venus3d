#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atlbase.h>

#ifdef _WINDLL
#define VENUSEXPORTER_API __declspec(dllexport)
#else
#define VENUSEXPORTER_API __declspec(dllimport)
#endif
