#include "stdafx.h"
#include "xml_helper.h"

static int_x parse_float(const char_16 * szText, int_x iLength, float_32 * floats, int_x iCount, char_16 chSplit = ',')
{
	int_x iIndex = 0;
	const char_16 * pText = szText;
	const char_16 * pEnd = szText + iLength;
	for(int_x cnt = 0; cnt < iCount && pText < pEnd; ++cnt)
	{
		const char_16 * pComma = textchr(pText, pEnd - pText, chSplit);
		if(pComma)
		{
			floats[iIndex++] = texttof32(pText, pComma - pText);
			pText = pComma + 1;
		}
		else
		{
			floats[iIndex++] = texttof32(pText, pEnd - pText);
			break;
		}
	}
	return iIndex;
}


float3 xmlh_float3(const textw & text)
{
	float3 ret;
	parse_float(text.buffer(), text.length(), ret.af, 3, L',');
	return ret;
}

float4 xmlh_float4(const textw & text)
{
	float4 ret;
	parse_float(text.buffer(), text.length(), ret.af, 4, L',');
	return ret;
}
