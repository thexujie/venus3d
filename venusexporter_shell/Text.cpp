#include "stdafx.h"
#include "Text.h"

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

int_x textformat_calc(const char_8 * format, ...)
{
	va_list args = NULL;
	va_start(args, format);
	int_x iLength = textformatargs_calc(format, args);
	va_end(args);
	return iLength;
}

int_x textformat_calc(const char_16 * format, ...)
{
	va_list args = NULL;
	va_start(args, format);
	int_x length = textformatargs_calc(format, args);
	va_end(args);
	return length;
}

int_x textformat(char_8 * buffer, int_x size, const char_8 * format, ...)
{
	va_list args = NULL;
	va_start(args, format);
	int_x length = textformatargs(buffer, size, format, args);
	va_end(args);
	return length;
}

int_x textformat(char_16 * buffer, int_x size, const char_16 * format, ...)
{
	va_list args = NULL;
	va_start(args, format);
	int_x length = textformatargs(buffer, size, format, args);
	va_end(args);
	return length;
}

int_x textformatargs_calc(const char_8 * format, void * args)
{
	return _vscprintf(format, (va_list)args) + 1;
}

int_x textformatargs_calc(const char_16 * format, void * args)
{
	return _vscwprintf(format, (va_list)args) + 1;
}

int_x textformatargs(char_8 * buffer, int_x size, const char_8 * format, void * args)
{
	return vsprintf_s(buffer, size, format, (va_list)args);
}

int_x textformatargs(char_16 * buffer, int_x size, const char_16 * format, void * args)
{
	return vswprintf_s(buffer, size, format, (va_list)args);
}

bool isprintchar(char_32 ch)
{
	if(ch == 0x7F)
		return false;
	else if(ch >= 0xFF)
		return true;
	else if(ch == L'\t')
		return true;
	else if(ch >= L' ' && ch <= L'~')
		return true;
	else
		return false;
}

bool ansitounicode(const char_8 * src, int_x src_length, char_16 * dst, int_x dst_size)
{
	uint_x uiSuit = 0;
	if(src_length < 0)
		src_length = textlen(src);
	mbstowcs_s((size_t *)&uiSuit, dst, dst_size, src, src_length);
	return true;
}

bool unicodetoansi(const char_16 * src, int_x src_length, char_8 * dst, int_x dst_size)
{
	uint_x uiSuit = 0;
	if(src_length < 0)
		src_length = textlen(src);
	wcstombs_s((size_t *)&uiSuit, dst, dst_size, src, src_length);
	return true;
}
