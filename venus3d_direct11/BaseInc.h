#pragma once

#if defined _WINDLL && !defined VENUS_NO_EXPORT
#define VDX11_API __declspec (dllexport)
#else
#define VDX11_API
#endif
