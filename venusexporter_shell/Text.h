#pragma once

#ifdef _WIN64
typedef long long int int_x;
typedef unsigned long long int uint_x;
#else
typedef int int_x;
typedef unsigned int uint_x;
#endif
typedef int int_32;

typedef char char_8;
typedef wchar_t char_16;
typedef int char_32;

template<typename T>
const T & Min(const T & a, const T & b)
{
	return a > b ? b : a;
}
/*!@brief ���� pSrc�е�iCount��EleT������ΪiSize��EleT��pArray�С�*/
template<typename EleT>
void arraycpy(EleT * pArray, int_x iSize, const EleT * pSrc, int_x iCount)
{
	int_x iLength = iSize > iCount ? iCount : iSize;
	for(int_x cnt = 0; cnt != iLength; ++cnt)
		pArray[cnt] = pSrc[cnt];
}


template<typename CharT>
void textempty(CharT * szText, int_x iSize = -1)
{
	if(szText && iSize)
		szText[0] = 0;
}

template<typename CharT>
bool textisempty(CharT * szText, int_x iSize = -1)
{
	return !iSize || !szText || !szText[0];
}

/**
 * @brief �����ı��ĳ��ȡ�
 * @param pText Ҫ���㳤�ȵ��ı���
 * @param iSize �����ı��Ļ�������С�����ı���󳤶�Ϊ iSize - 1��Ĭ��Ϊ-1��
 *              ���Ϊ�������������̲������ı���������С����
 * @return ���ؼ�����ı��ĳ��ȡ�
 */
template<typename CharT>
int_x textlen(const CharT * szText, int_x iSize = -1)
{
	if(!szText || !iSize)
		return 0;

	int_x iLength = 0;
	while(*szText++ && --iSize)
		++iLength;
	return iLength;
}


/**
 * @brief ��Դ�ַ��ܵ�ָ�����ȸ��Ƶ�Ŀ���ַ������������ֹ��(����Ŀ���ַ�����СΪ0)��
 * @param szText Ŀ���ַ�����
 * @param iSize Ŀ���ַ����Ĵ�С��������ֹ����������Ŀ���ַ�����������
 * @param szSrc Դ�ַ�����
 * @param iLength Ҫ�������ַ��ĸ�������������ֹ�������Ϊ�������򿽱�ֱ����ֹ��Ϊֹ��
 *                ���� iLength ָ���ĳ��ȴ��� pSrc ����ʵ���ȣ���Ҳֻ�´��ֱ����ֹ��Ϊֹ��
 * @return ���ؿ������ַ��ĸ�������������ֹ����
 */
template<typename CharT>
int_x textcpy(CharT * szText, int_x iSize, const CharT * szSrc, int_x iLength = -1)
{
	if(!iSize)
		return 0;
	else if(iSize == 1)
	{
		*szText = 0;
		return 0;
	}
	else
	{
		if(iLength < 0)
			iLength = Min(textlen(szSrc), iSize - 1);
		else
		{
			int_x iLengthMax = textlen(szSrc, iLength + 1);
			if(iLength > iLengthMax)
				iLength = Min(iLengthMax, iSize - 1);
			else
				iLength = Min(iLength, iSize - 1);
		}


		if(iLength)
			arraycpy(szText, iSize, szSrc, iLength);
		szText[iLength] = 0;
		return iLength;
	}
}

// -------------------------------------------------------------------------------------------------
// -------------------- ���ı���������ɾ���ĵĲ��� -----------------------------------------------------
// -------------------------------------------------------------------------------------------------
/**
* @brief ��Դ�ַ��ܵ�ָ�����ȸ��Ƶ�Ŀ���ַ������������ֹ����
* @param pText Ŀ���ַ�����
* @param iSize Ŀ���ַ����Ĵ�С��������ֹ����������Ŀ���ַ�����������
* @param pSrc Դ�ַ�����
* @param iCount Ҫ�������ַ��ĸ�������������ֹ����
* @see TextCopy
* @note ��ͬ�� TextCopy��TextCopy �������ֹ���ġ�
*/
template<typename CharT>
void copytext(CharT * pText, int_x iSize, const CharT * pSrc, int_x iCount = -1)
{
	if(iCount < 0)
		iCount = textlen(pSrc);

	if(iCount)
	{
		if(iSize < 0)
			iSize = 0;
		iCount = Min(iCount, iSize);
		if(iCount)
			arraycpy(pText, iSize, pSrc, iCount);
	}
}

template<typename CharT>
const CharT * textchr(const CharT * szText, int_x iLength, const CharT & ch)
{
	if(!szText || !iLength)
		return nullptr;

	while(*szText && iLength--)
	{
		if(*szText == ch)
			return szText;
		else
			++szText;
	} 
	return nullptr;
}

template<typename CharT>
const CharT * textrchr(const CharT * szText, int_x iLength, const CharT & ch)
{
	if(!szText || !iLength)
		return nullptr;

	if(iLength < 0)
		iLength = textlen(szText);

	const CharT * pLast = szText + (iLength - 1);
	const CharT * pEnd = szText - 1;

	while(iLength-- && pLast > pEnd)
	{
		if(*pLast == ch)
			return pLast;
		else
			--pLast;
	} 
	return nullptr;
}

template<typename CharT>
int_x textcmp(const CharT * szText, int_x iLength, const CharT * szPattern, int_x iPatternLength)
{
	if(!szText && !szPattern)
		return 0;
	if(!iLength && !iPatternLength)
		return 0;

	CharT ch1, ch2;
	while(iLength-- && iPatternLength--)
	{
		ch1 = *szText++;
		ch2 = *szPattern++;

		if(ch1 != ch2)
			return ch1 - ch2;
		if(ch1 == 0)
			return 0;
	}
	return 0;
}

template<typename CharT>
const CharT * textstr(const CharT * szText, int_x iLength, const CharT * szPattern, int_x iPatternLength)
{
	if(!szText || !szPattern)
		return nullptr;
	if(!iLength && !iPatternLength)
		return nullptr;

	const CharT * pText = szText;
	for(; (pText = textchr(pText, iLength, *szPattern)) != 0; ++pText, --iLength)
	{
		if(textcmp(pText, iLength, szPattern, iPatternLength) == 0)
			return pText;
	}
	return nullptr;
}

template<typename CharT>
const CharT * textrstr(const CharT * szText, int_x iLength, const CharT * szPattern, int_x iPatternLength)
{
	if(!szText || !szText[0] || !iLength || !szPattern || !szPattern[0] || !iPatternLength)
		return false;

	if(iLength < 0)
		iLength = textlen(szText);
	if(iPatternLength < 0)
		iPatternLength = textlen(szPattern);

	if(iLength < iPatternLength)
		return false;

	bool bEqual = false;
	const CharT * pText = szText + (iLength - iPatternLength);
	while(pText >= szText)
	{
		bEqual = true;
		for(int_x cnt = 0; cnt != iPatternLength; ++cnt)
		{
			if(pText[cnt] != szPattern[cnt])
			{
				bEqual = false;
				break;
			}
		}
		if(bEqual)
			return pText;
		else
			--pText;
	}
	return nullptr;
}

template<typename CharT>
int_x textrplchr(CharT * szText, int_x iSize, const CharT & src, const CharT & dst)
{
	if(!szText || !szText[0] || !iSize)
		return 0;
	
	int_x iCount = 0;
	while(*szText && iSize--)
	{
		if(*szText == src)
		{
			*szText = dst;
			++iCount;
		}
		else{}
		++szText;
	}
	return iCount;
}

template<typename CharT>
bool textbegwith(const CharT * szText, int_x iLength, const CharT & ch)
{
	if(iLength < 0)
		iLength = textlen(szText);

	if(!iLength)
		return false;
	else
		return szText[0] == ch;
}

template<typename CharT>
bool textbegwith(const CharT * szText, int_x iLength, const CharT * szPattern, int_x iPatternLength)
{
	if(iLength < 0)
		iLength = textlen(szText);
	if(iPatternLength < 0)
		iPatternLength = textlen(szPattern);

	if(!iLength || !iPatternLength || iLength < iPatternLength)
		return false;

	if(textcmp(szText, iPatternLength, szPattern, iPatternLength) == 0)
		return true;
	else
		return false;
}

template<typename CharT>
bool textendwith(const CharT * szText, int_x iLength, const CharT & ch)
{
	if(iLength < 0)
		iLength = textlen(szText);

	if(!iLength)
		return false;
	else
		return szText[iLength - 1] == ch;
}

template<typename CharT>
bool textendwith(const CharT * szText, int_x iLength, const CharT * szPattern, int_x iPatternLength)
{
	if(iLength < 0)
		iLength = textlen(szText);
	if(iPatternLength < 0)
		iPatternLength = textlen(szPattern);

	if(!iLength || !iPatternLength || iLength < iPatternLength)
		return false;

	const CharT * pText = szText + (iLength - iPatternLength);
	if(textcmp(pText, iPatternLength, szPattern, iPatternLength) == 0)
		return true;
	else
		return false;
}

template<typename CharT>
int_x textcat(CharT * szDst, int_x iSize, const CharT * szSrc, int_x iLength)
{
	if(iLength < 0)
		iLength = textlen(szSrc);
	if(!szDst || iSize <= 1 || !szSrc || !iLength)
		return 0;
	else
	{
#ifdef _DEBUG
		bool bFoundNull = false;
		for(CharT * pTemp = szDst, * pEnd = szDst + iSize; pTemp < pEnd; ++pTemp)
		{
			if(*pTemp == 0)
			{
				bFoundNull = true;
				break;
			}
		}
#endif
		int_x iCount = 0;
		--iSize;
		while(*szDst && iSize)
		{
			++szDst;
			--iSize;
		}

		while(iSize-- && *szSrc && iLength--)
		{
			*szDst++ = *szSrc++;
			++iCount;
		}

		*szDst = 0;
		return iCount;
	}
}

template<typename CharT>
int_x textcompose(CharT * szDst, int_x iSize, 
				const CharT * szSrcA, int_x iLengthA,
				const CharT * szSrcB, int_x iLengthB)
{
	if(!szDst || iSize <= 0)
		return 0;
	else if(iSize == 1)
	{
		*szDst = 0;
		return 0;
	}
	if(!szSrcA || !iLengthA)
		return textcpy(szDst, iSize, szSrcB, iLengthB);
	else if(!szSrcB || !iLengthB)
		return textcpy(szDst, iSize, szSrcA, iLengthA);
	else
	{
		int_x iLength = 0;
		while(iSize > 1 && iLengthA && *szSrcA)
		{
			*szDst++ = *szSrcA++;
			--iSize;
			--iLengthA;
			++iLength;
		}
		while(iSize > 1 && iLengthB && *szSrcB)
		{
			*szDst++ = *szSrcB++;
			--iSize;
			--iLengthB;
			++iLength;
		}
		*szDst = 0;
		return iLength;
	}
}


template<typename CharT>
int_x textcomposeex(CharT * szDst, int_x iSize,
				  const CharT ** pszSrc, int_x * piLength, int_x iCount)
{
	if(!szDst || iSize <= 0)
		return 0;
	else if(iSize == 1)
	{
		*szDst = 0;
		return 0;
	}
	else
	{
		int_x iLength = 0;
		int_x iSrcLength = 0;
		const CharT * pSrc = nullptr;
		for(int_x cnt = 0; cnt != iCount && iSize > 1; ++cnt)
		{
			iSrcLength = piLength[cnt];
			pSrc = pszSrc[cnt];

			while(iSize > 1 && iSrcLength && *pSrc)
			{
				*szDst++ = *pSrc++;
				--iSize;
				--iSrcLength;
				++iLength;
			}
		}
		*szDst = 0;
		return iLength;
	}
}

template<typename CharT>
int_x texttrim(CharT * szText, int_x iLength = -1, const CharT * szTrim = nullptr, int_x iTrimLength = -1)
{
	if(iLength < 0)
		iLength = textlen(szText);
	if(!iLength)
		return 0;

	if(szTrim)
	{
		if(iTrimLength < 0)
			iTrimLength = textlen(szTrim);
		if(!iTrimLength)
			return 0;
		CharT * pLeft = szText;
		CharT * pEnd = pLeft + iLength;
		CharT ch = 0;

		while(pLeft != pEnd)
		{
			ch = *pLeft;
			if(textchr(szTrim, iTrimLength, ch))
				++pLeft;
			else
				break;
		}

		if(pLeft == pEnd)
		{
			*szText = 0;
			return iLength;
		}
		else
		{
			CharT * pRight = pEnd;
			while(pRight != pLeft)
			{
				ch = *(pRight - 1);
				if(textchr(szTrim, iTrimLength, ch))
					--pRight;
				else
					break;
			}

			int_x iRet = iLength - (pRight - pLeft);
			CharT * pDst = szText;
			while(pLeft != pRight)
				*pDst++ = *pLeft++;
			*pDst = 0;
			return iRet;
		}
	}
	else
	{
		CharT * pLeft = szText;
		CharT * pEnd = pLeft + iLength;
		CharT ch = 0;
		while(pLeft != pEnd)
		{
			ch = *pLeft;
			if(ch == ' ' || ch == '\t')
				++pLeft;
			else
				break;
		}

		if(pLeft == pEnd)
		{
			*szText = 0;
			return iLength;
		}
		else
		{
			CharT * pRight = pEnd;
			while(pRight != pLeft)
			{
				ch = *(pRight - 1);
				if(ch == ' ' || ch == '\t')
					--pRight;
				else
					break;
			}

			int_x iRet = iLength - (pRight - pLeft);
			CharT * pDst = szText;
			while(pLeft != pRight)
				*pDst++ = *pLeft++;
			*pDst = 0;
			return iRet;
		}
	}
	return 0;
}














// ---------------------------------------------- �ļ�·����ء�
const int_x MAX_FILE_PATH = 260;

/**
 * @brief ����·���е� "./" �� "../" ��ǡ�
 * "./" ��ʾ��ǰĿ¼��
 * "../" ��ʾ��һ��Ŀ¼��
 * �ú����������еķ�б�� '\' �����滻Ϊ�� б�� '/��
 */
template<typename CharT>
int_x textrepairpath(CharT * szPath, int_x iLength)
{
	if(iLength < 0)
		iLength = textlen(szPath);
	if(!iLength)
		return -1;

	int_x iPoint = 0;

	CharT * pBeg = szPath;
	CharT * pEnd = pBeg + iLength;

	CharT * pPath = pBeg;
	CharT * pSlash = pBeg;
	CharT * pWrite = pBeg;
	CharT ch = 0;

	while(pPath != pEnd)
	{
		ch = *pPath;
		if(ch == '.')
			++iPoint;
		else if(ch == '/')
		{
			// ���Ա�ʾ��ǰĿ¼�� "./"��
			if(iPoint == 1)
			{
				iPoint = 0;
			}
			else if(iPoint == 2)
			{
				if(pSlash == pBeg)
					break; // û�и��ϲ��Ŀ¼�ˡ�

				--pSlash;
				// ��������һ��'/'��
				while(pSlash != pBeg)
				{
					if(*pSlash == '/')
						break;
					else
						--pSlash;
				}

				if(pSlash != pBeg)
					pWrite = pSlash + 1;
				else
					pWrite = pSlash;
				iPoint = 0;
			}
			else
			{
				if(iPoint > 2)
				{
					if(pWrite < pPath)
					{
						for(int_x cnt = 0; cnt != iPoint; ++cnt)
							pWrite[cnt] = '.';
						pWrite += iPoint;
						iPoint = 0;
						*pWrite = ch;
						++pWrite;
					}
					iPoint = 0;
				}
				else
				{
					if(pWrite < pPath)
						*pWrite = ch;
					++pWrite;
				}
				pSlash = pPath;
			}
		}
		else
		{
			if(pWrite < pPath)
			{
				for(int_x cnt = 0; cnt != iPoint; ++cnt)
					pWrite[cnt] = '.';
				pWrite += iPoint;
				iPoint = 0;
				*pWrite = ch;
				++pWrite;
			}
			else
			{
				pWrite += iPoint;
				++pWrite;
			}
		}
		++pPath;
	}

	if(pWrite < pEnd)
		*pWrite = 0;
	return pWrite - szPath;
}

/**
 * @brief ���ļ���·�������Ŀ¼��������Ŀ¼��
 * �ڲ���� '\' �滻�� '/'��
 */
template<typename CharT>
int_x textmakepath(CharT * szPath, int_x iSize, 
				  const CharT * szDirectory, int_x iDirectoryLength, 
				  const CharT * szRelative, int_x iRelativeLength)
{
	textempty(szPath, iSize);
	if(!iSize)
		return 0;

	if(iDirectoryLength < 0)
		iDirectoryLength = textlen(szDirectory, MAX_FILE_PATH);
	if(iRelativeLength < 0)
		iRelativeLength = textlen(szRelative, MAX_FILE_PATH);

	const int_x COUNT = MAX_FILE_PATH * 2 + 4;
	CharT szTemp[COUNT] = {0};

	textcpy(szTemp, COUNT, szDirectory, iDirectoryLength);
	textrplchr(szTemp, iDirectoryLength, _T('\\'), _T('/'));

	// ��ֹ '/' �ظ���
	bool bEndL = szTemp[iDirectoryLength - 1] == '/';
	bool bEndR = szRelative[0] == '/';
	if(bEndL && bEndR)
	{
		--iDirectoryLength;
	}
	else if(!bEndL && !bEndR)
	{
		szTemp[iDirectoryLength] = '/';
		++iDirectoryLength;
	}
	else{}

	textcpy(szTemp + iDirectoryLength, COUNT - iDirectoryLength, szRelative, iRelativeLength);
	textrplchr(szTemp + iDirectoryLength, iRelativeLength, _T('\\'), _T('/'));

	int_x iLength = iDirectoryLength + iRelativeLength;
	iLength = textrepairpath(szTemp, iLength);
	if(iLength >= 0)
		return textcpy(szPath, iSize, szTemp, iLength);
	else
		return 0;
}

template<typename CharT>
void textsplitpath(const CharT * szPath, int_x iLength, 
				   CharT * szDrive, int_x iDriveSize,
				   CharT * szFolder, int_x iFolderSize,
				   CharT * szName, int_x iNameSize,
				   CharT * szExt, int_x iExtSize,
				   CharT * szFileName, int_x iFileNameSize,
				   int_x * pDriveLength = nullptr, 
				   int_x * pDirLength = nullptr, 
				   int_x * pNameLength = nullptr, 
				   int_x * pExtLength = nullptr, 
				   int_x * pFileNameLength = nullptr)
{
	textempty(szDrive, iDriveSize);
	textempty(szFolder, iFolderSize);
	textempty(szName, iNameSize);
	textempty(szExt, iExtSize);
	textempty(szFileName, iFileNameSize);

	if(iLength < 0)
		iLength = textlen(szPath);

	if(!iLength || !szPath)
		return;

	const CharT * pPath = szPath;
	const CharT * pEnd = pPath + iLength;
	int_x iDriveLength = 0, iDirLength = 0, iNameLength = 0, iExtLength = 0, iFileNameLength = 0;

	if((!szDrive && iDriveSize) || (szDrive && !iDriveSize) ||
		(!szFolder && iFolderSize) || (szFolder && !iFolderSize) ||
		(!szName && iNameSize) || (szName && !iNameSize) ||
		(!szExt && iExtSize) || (szExt && !iExtSize))
		return;

	// ������
	const CharT * pTemp = pPath;
	const CharT * pColon = nullptr;
	while(pTemp != pEnd)
	{
		if(*pTemp == ':')
		{
			pColon = pTemp;
			++pTemp;
			break;
		}
		else
			++pTemp;
	}

	if(!pColon)
		pTemp = pPath;
	else
		iDriveLength = textcpy(szDrive, iDriveSize, pPath, (pColon - pPath) + 1);

	// Ŀ¼
	pPath = pTemp;
	const CharT * pSlash = nullptr;
	while(pTemp != pEnd)
	{
		if(*pTemp == '\\' || *pTemp == '/')
			pSlash = pTemp;
		++pTemp;
	}

	if(!pSlash)
		pTemp = pPath;
	else
	{
		iDirLength = textcpy(szFolder, iFolderSize, szPath, (pSlash - szPath) + 1);
		pTemp = pSlash + 1;
	}

	// ����
	pPath = pTemp;
	iFileNameLength = textcpy(szFileName, iFileNameSize, pPath, pEnd - pPath);
	const CharT * pDot = nullptr;
	while(pTemp != pEnd)
	{
		if(*pTemp == '.')
			pDot = pTemp;
		++pTemp;
	}

	if(!pDot)
		iNameLength = textcpy(szName, iNameSize, pPath, pEnd - pPath);
	else
	{
		iNameLength = textcpy(szName, iNameSize, pPath, pDot - pPath);
		iExtLength = textcpy(szExt, iExtSize, pDot, pEnd - pDot);
	}

	if(pDriveLength) *pDriveLength = iDriveLength;
	if(pDirLength) *pDirLength = iDirLength;
	if(pNameLength) *pNameLength = iNameLength;
	if(pExtLength) *pExtLength = iExtLength;
	if(pFileNameLength) *pFileNameLength = iFileNameLength;
}

template<typename CharT>
int_x textsplitpath_drive(const CharT * szPath, int_x iLength, CharT * szDrive, int_x iDriveSize)
{
	int_x iDriveLength = 0;
	textsplitpath<CharT>(szPath, iLength,
						 szDrive, iDriveSize, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0,
						 &iDriveLength, nullptr, nullptr, nullptr, nullptr);
	return iDriveLength;
}

template<typename CharT>
int_x textsplitpath_folder(const CharT * szPath, int_x iLength, CharT * szFolder, int_x iFolderSize)
{
	int_x iFolderLength = 0;
	textsplitpath<CharT>(szPath, iLength,
						 nullptr, 0, szFolder, iFolderSize, nullptr, 0, nullptr, 0, nullptr, 0,
						 nullptr, &iFolderLength, nullptr, nullptr, nullptr);
	return iFolderLength;
}

template<typename CharT>
int_x textsplitpath_name(const CharT * szPath, int_x iLength, CharT * szName, int_x iNameSize)
{
	int_x iNameLength = 0;
	textsplitpath<CharT>(szPath, iLength,
						 nullptr, 0, nullptr, 0, szName, iNameSize, nullptr, 0, nullptr, 0,
						 nullptr, nullptr, &iNameLength, nullptr, nullptr);
	return iNameLength;
}

template<typename CharT>
int_x textsplitpath_ext(const CharT * szPath, int_x iLength, CharT * szExt, int_x iExtSize)
{
	int_x iExtLength = 0;
	textsplitpath<CharT>(szPath, iLength,
						 nullptr, 0, nullptr, 0, nullptr, 0, szExt, iExtSize, nullptr, 0,
						 nullptr, nullptr, nullptr, &iExtLength, nullptr);
	return iExtLength;
}

template<typename CharT>
int_x textsplitpath_filename(const CharT * szPath, int_x iLength, CharT * szFileName, int_x iFileNameSize)
{
	int_x iFileNameLength = 0;
	textsplitpath<CharT>(szPath, iLength,
						 nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0, szFileName, iFileNameSize,
						 nullptr, nullptr, nullptr, nullptr, &iFileNameLength);
	return iFileNameLength;
}

/**
 * @brief ����һ������ URL �е�Э�顢�������ļ������֡�
 */
template<typename CharT>
void textspliturl(const CharT * szUrl, int_x iLength, 
				  CharT * szProtocol, int_x iProtocolSize,
				  CharT * szDomain, int_x iDomainSize,
				  CharT * szFile, int_x iFileSize,
				  int_x * piProtocolLength = nullptr,
				  int_x * piDomainLength = nullptr,
				  int_x * piFileLength = nullptr)
{
	textempty(szProtocol, iProtocolSize);
	textempty(szDomain, iDomainSize);
	textempty(szFile, iFileSize);

	const CharT * pUrl = szUrl;

	int_x iProtocolLength = 0, iDomainLength = 0, iFileLength = 0;
	const char * pSlashSlash = textstr(pUrl, iLength, "//", 2);
	if(pSlashSlash)
	{
		iProtocolLength = textcpy(szProtocol, iProtocolSize, pUrl, pSlashSlash - pUrl);
		pUrl = pSlashSlash + 2;
		const char * pSlash = textchr(pUrl, iLength, '/');
		if(pSlash)
		{
			iDomainLength = textcpy(szDomain, iDomainSize, pUrl, pSlash - pUrl);
			++pSlash;
			iFileLength = textcpy(szFile, iFileSize, pSlash, (szUrl + iLength) - pSlash);
		}
		else{}
	}
	else{}
	if(piProtocolLength) *piProtocolLength = iProtocolLength;
	if(piDomainLength) *piDomainLength = iDomainLength;
	if(piFileLength) *piFileLength = iFileLength;
}

template<typename CharT>
void textspliturlprotocol(const CharT * szUrl, int_x iLength,
						  CharT * szProtocol, int_x iProtocolSize,
						  int_x * piProtocolLength = nullptr)
{
	textspliturl<CharT>(szUrl, iLength, szProtocol, iProtocolSize, nullptr, 0, nullptr, 0, piProtocolLength, nullptr, nullptr);
}

template<typename CharT>
void textspliturldomain(const CharT * szUrl, int_x iLength,
						CharT * szDomain, int_x iDomainSize,
						int_x * piDomainLength = nullptr)
{
	textspliturl<CharT>(szUrl, iLength, nullptr, 0, szDomain, iDomainSize, nullptr, 0, nullptr, piDomainLength, nullptr);
}

template<typename CharT>
void textspliturlfile(const CharT * szUrl, int_x iLength,
					  CharT * szFile, int_x iFileSize,
					  int_x * piFileLength = nullptr)
{
	textspliturl<CharT>(szUrl, iLength, nullptr, 0, nullptr, 0, szFile, iFileSize, nullptr, nullptr, piFileLength);
}


/// ���ı�ת��Ϊ����
template<typename CharT, typename IntType>
IntType texttoi(const CharT * text, int_x length = -1, int_x scale = 0)
{
	if(scale <= 0)
		scale = 10;

	if(scale < 2 || scale > 35 || !length || !text)
		return 0;

	if(length < 0)
		length = textlen(text);

	while(length && *text == ' ')
	{
		++text;
		--length;
	}
	IntType sign = 1;
	IntType interger = 0;
	IntType decimal = 0;
	CharT ch = 0;

	if(text[0] == L'-')
	{
		sign = (IntType)-1;
		++text;
	}
	const CharT * curr = text;

	// ȥ�� 16 ���� 0x(0X)
	if(length > 2 && *curr == '0' && (*(curr + 1) == 'x' || *(curr + 1) == 'X'))
	{
		scale = 16;
		curr += 2;
	}

	while(length--)
	{
		ch = *curr++;
		if(!ch)
			break;

		if(ch >= '0' && ch <= '9')
			decimal = ch - '0';
		else if(ch >= 'A' && ch <= 'Z')
			decimal = ch - 'A' + 10;
		else if(ch >= 'a' && ch <= 'z')
			decimal = ch - 'A' + 10;
		else
			break;

		if(decimal >= scale)
			break;

		interger = interger * scale + decimal;
	}
	return interger * sign;
}

template<typename CharT>
int_x texttoix(const CharT * text, int_x length = -1, int_x scale = 0)
{
	return texttoi<CharT, int_x>(text, length, scale);
}

template<typename CharT>
int_32 texttoi32(const CharT * text, int_x length = -1, int_x scale = 0)
{
	return texttoi<CharT, int_32>(text, length, scale);
}

/// ���ı�ת��Ϊ������
template<typename CharT, typename FloatType>
FloatType texttof(const CharT * text, int_x length = -1)
{
	if(!length)
		return (FloatType)0;
	if(text[0] == '-')
		return -texttof<CharT, FloatType>(text + 1, length - 1);
	else
	{
		// radix * e^index
		FloatType radix = (FloatType)10;
		FloatType index = (FloatType)0;
		char_8 ch = 0;
		// ��������
		while(ch = *text++ && length--)
		{
			if(ch == '.' || ch < '0' || ch > '9')
			{
				// С������
				while(ch = *text++)
				{
					if(ch < '0' || ch > '9')
						break;

					index += (FloatType)(ch - '0') / radix;
					radix *= 10;
				}

				// ָ����ʽ
				if((ch == 'E' || ch == 'e') && (index != 0))
				{
					int_x index_num = texttoix(text, -1, 10);
					if(index_num >= 0)
					{
						while(index_num--)
							index *= 10;
					}
					else
					{
						while(index_num++)
							index /= 10;
					}
				}
				break;
			}
			index *= 10;
			index += ch - '0';
		}
		return index;
	}
}

/// ������ת��Ϊ�ı�
template<typename CharT, typename IntType>
int_x textfromi(CharT * text, int_x size, IntType value, int_x scale = 0, bool upper = false)
{
	if(scale <= 0)
		scale = 10;

	// ���Ʋ��Ϸ�
	if(scale < 2 || scale > 35)
	{
		if(text && size)
			text[0] = 0;
		return 0;
	}
	int_x length = 0;
	bool positive = true;
	if(value < 0)
	{
		value = value * (IntType)(-1);
		positive = false;
		++length;
	}

	IntType temp = value;
	do
	{
		++length;
		temp /= scale;
	}
	while(temp);

	if(!text)
		return length;

	if(length >= size)
	{
		if(text && size)
			text[0] = 0;
		return length;
	}
	else {}

	int_x index = length;
	if(!positive)
	{
		text[0] = '-';
	}

	CharT chA = upper ? 'A' : 'a';
	IntType number = 0;
	do
	{
		number = value % scale;
		value /= scale;

		if(text)
		{
			if(number > 9)
				text[--index] = (CharT)(chA + (number - 10));
			else
				text[--index] = (CharT)('0' + number);
		}
	}
	while(value);

	text[length] = '\0';
	return length;
}

/// �ж������ı��Ƿ����
template<typename CharT>
bool textequalex(const CharT * text, int_x length, const CharT * pattern, int_x pattern_length, bool case_sensitive = true)
{
	if(!text || !pattern)
		return false;

	if(length < 0)
		length = textlen<CharT>(text);
	if(pattern_length < 0)
		pattern_length = textlen<CharT>(pattern);

	if(length != pattern_length)
		return false;
	else
	{
		if(case_sensitive)
		{
			while(length--)
			{
				if(*text != *pattern)
					return false;

				++text;
				++pattern;
			}
			return true;
		}
		else
		{
			while(length--)
			{
				if(*text != *pattern)
				{
					if((*text + 32 != *pattern) && (*text != *pattern + 32))
						return false;
					else {}
				}

				++text;
				++pattern;
			}
			return true;
		}
	}
}

//! @brief �ж������ı��Ƿ����
template<typename CharT>
bool textequal(const CharT * text, const CharT * pattern)
{
	return textequalex(text, -1, pattern, -1, true);
}

/// �ж������ı��Ƿ񲿷����
template<typename CharT>
bool textnequal(const CharT * text, int_x legnth, const CharT * pattern, int_x pattern_length, bool case_sensitive = true)
{
	if(!text || !pattern)
		return false;

	if(legnth < 0)
		legnth = textlen<CharT>(text);
	if(pattern_length < 0)
		pattern_length = textlen<CharT>(pattern);

	if(pattern_length > legnth)
		return false;
	else
	{
		if(case_sensitive)
		{
			while(pattern_length--)
			{
				if(*text != *pattern)
					return false;

				++text;
				++pattern;
			}
			return true;
		}
		else
		{
			while(pattern_length--)
			{
				if(*text != *pattern)
				{
					if((*text + 32 != *pattern) && (*text != *pattern + 32))
						return false;
					else {}
				}

				++text;
				++pattern;
			}
			return true;
		}
	}
}

/**
* @brief �Ƴ��ı��е�һ���ַ���
* @param pText Ҫ�������ı���
* @param iLength Ҫ�������ı��ĳ��ȡ�
* @param iSize Ҫ�������ı���������
* @param iIndex �Ƴ����ַ�������
*/
template<typename CharT>
void textremovech(CharT * text, int_x length, int_x index)
{
	if(index >= 0 && index < length)
	{
		if(index != length - 1)
			arraymove(text + index, length - index, text + index + 1, length - (index + 1));
		text[length - 1] = 0;
	}
}

/**
* @brief �Ƴ��ı���һ���֡�
* @param pText Ҫ�������ı���
* @param iLength Ҫ�������ı��ĳ��ȡ�
* @param iSize Ҫ�������ı���������
* @param iIndex �Ƴ����ֵ���ʼ������
* @param iCount �Ƴ����ַ�������
*/
template<typename CharT>
void textremovestr(CharT * text, int_x length, int_x index, int_x count)
{
	if(index >= 0 && index < length &&
	   count > 0 && index + count <= length)
	{
		arraymove(text + index, length - index, text + index + count, length - (index + count));
		text[length - count] = 0;
	}
}

/**
* @brief ���ı��в���һ���ַ���
* @param pText Ҫ�������ı���
* @param iLength Ҫ�������ı��ĳ��ȡ�
* @param iSize Ҫ�������ı���������
* @param iIndex �����λ��������
*/
template<typename CharT>
void textinsert(CharT * text, int_x length, int_x size, int_x index, const CharT & ch)
{
	if(length <= size &&
	   index >= 0 && index <= length)
	{
		if(index != length)
			arraymove(text + index + 1, size - index - 1, text + index, length - index);
		text[index] = ch;
		text[length + 1] = 0;
	}
}

/**
* @brief ���ı��в�����һ���ı���
* @param pText Ҫ�������ı���
* @param iLength Ҫ�������ı��ĳ��ȡ�
* @param iSize Ҫ�������ı���������
* @param pSrc ������ı���
* @param iCount ������ַ����������С�� 0�������ȫ���ַ���
* @param iIndex �����λ��������
*/
template<typename CharT>
void textinsert(CharT * text, int_x length, int_x size, int_x index, const CharT * src, int_x src_length)
{
	if(src_length < 0)
		src_length = textlen(src);

	if(length <= size &&
	   index >= 0 && index <= length &&
	   length + src_length <= size)
	{
		arraymove(text + index + src_length, size - (index + src_length), text + index, length - index);
		arraycpy(text + index, size - index, src, src_length);
		text[length + src_length] = 0;
	}
}

/**
* @brief ����ָ�����ı��Ͳ�����ʽ��������Ļ�������С(������ֹ��)��
* @param szFormat ��ʽ���ַ�����
* @param ... ������
*/
int_x textformat_calc(const char_8 * format, ...);

/**
* @copydoc textformat_calc(const char_8 * szFormat, ...) ��
*/
int_x textformat_calc(const char_16 * format, ...);

/**
* @brief ����ָ�����ı��Ͳ����б��ʽ��������Ļ�������С(������ֹ��)��
* @param format ��ʽ���ַ�����
* @param args �����б�
*/
int_x textformatargs_calc(const char_8 * format, void * args);

/**
* @copydoc TextFormatCalcArgs(const char_8 * format, void * args) ��
*/
int_x textformatargs_calc(const char_16 * format, void * args);

/**
* @brief ���ַ�����ʽ����ָ���������ڡ�
* @param buffer Ŀ�껺������
* @param size Ŀ�껺�����Ĵ�С(������ֹ��)��
* @param format ��ʽ���ַ�����
* @param ... �����б�
*/
int_x textformat(char_8 * buffer, int_x size, const char_8 * format, ...);

/**
* @copydoc TextFormatCalcArgs(const char_16 * format, void * args);
*/
int_x textformat(char_16 * buffer, int_x size, const char_16 * format, ...);

/**
* @brief ���ַ�����ʽ����ָ���������ڡ�
* @param buffer Ŀ�껺������
* @param size Ŀ�껺�����Ĵ�С(������ֹ��)��
* @param format ��ʽ���ַ�����
* @param args �����б�ָ�롣
*/
int_x textformatargs(char_8 * buffer, int_x size, const char_8 * format, void * args);

/**
* @copydoc TextFormatArgs(char_8 * buffer, int_x size, const char_8 * format, void * args);
*/
int_x textformatargs(char_16 * buffer, int_x size, const char_16 * format, void * args);

/**
* @brief �ж��ַ��Ƿ��ǿ���ʾ�ġ�
* ����ʾ�ַ����������š���Сд��ĸ�������Լ�����0xFF�������ַ���
*/
bool isprintchar(char_32 ch);

bool ansitounicode(const char_8 * src, int_x src_length, char_16 * dst, int_x dst_size);
bool unicodetoansi(const char_16 * src, int_x src_length, char_8 * dst, int_x dst_size);

