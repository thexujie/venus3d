#include "stdafx.h"
#include "venusexporter.h"
#include "CVmpExporter.h"
#include "CVmpWindow.h"

// Dummy function for progress bar
DWORD WINAPI InterfaceCallBack(void * arg)
{
	return 0;
}


int_32 ExportToVmp(const char_16 * szFileName, ExpInterface * pei, Interface * pi, BOOL bSuppressPrompts, DWORD dwOptions)
{
	pi->ProgressStart(L"Venus Exporter...", TRUE, InterfaceCallBack, nullptr);

	const char_16 * szExt = textprch(szFileName, -1, L'.');

	ExportModeE eMode = ExportModeMesh;
	if(textequalex(szExt, -1, L".xemesh", 5, false))
		eMode = ExportModeMesh;
	else if(textequalex(szExt, -1, L".xebone", 4, false))
		eMode = ExportModeAnim;
	else {}

	setlocale(0, "");

	CVmpWindow vw;
	vw.CreateHost();
	IWindow * pWindow = (IWindow *)vw.GetHost()->GetInterface(OID_IWindow);
	if(pWindow)
		pWindow->SetParentHandle(pi->GetMAXHWnd());

	vw.Show(ShowModeNormal, HostInitPosCenterParent);
	if(vw.RunDialog() == DialogResultOk)
	{
		eMode = vw.GetMode();
		bool bExportSelected = (dwOptions & SCENE_EXPORT_SELECTED) != 0;
		vertexformat_e eVertexFormat = vw.GetVertexFormat();
		bool bLocalCoord = vw.IsUseLocalCoord();

		char_16 szFile[MAX_FILE_PATH];
		textcpy(szFile, MAX_FILE_PATH, szFileName, szExt - szFileName);
		switch(eMode)
		{
		case ExportModeMesh:
			textcat(szFile, MAX_FILE_PATH, L".xemesh", 5);
			break;
		case ExportModeAnim:
			textcat(szFile, MAX_FILE_PATH, L".xeact", 4);
			break;
		default:
			break;
		}
		textlower(szFile, -1);

		CFileStream fSave(szFile, StreamModeWrite | StreamModeExCreateAlways);
		textcat(szFile, MAX_FILE_PATH, L".csv", 4);
		CFileStream fLog(szFile, StreamModeWrite | StreamModeExCreateAlways);

		if(!fSave.IsValid() || !fLog.IsValid())
		{
			CMessageBox::ShowMessage(nullptr, L"Some files are occupied by others.", L"Invalid File Access", L"Venus Exporter");
			pi->ProgressEnd();
			return IMPEXP_CANCEL;
		}
		CBufferedOutputStream bosFile(&fSave, 1024);
		CBufferedOutputStream bosLog(&fLog, 1024);
		//CTextOutputStream tos(&bos);
		CDataOutputStream dosFile(&bosFile);
		CTextOutputStream tosLog(&bosLog);

		CVmpExporter exporter(pi, &tosLog, 0, bExportSelected);
		pei->theScene->EnumTree(&exporter);
		try
		{
			exporter.Parse(bLocalCoord);
			switch(eMode)
			{
			case ExportModeMesh:
				exporter.ExportMesh(&dosFile, eVertexFormat);
				break;
			case ExportModeAnim:
				exporter.ExportAnim(&dosFile, 30);
				break;
			default:
				break;
			}
		}
		catch (...)
		{
			log2(L"Failed to parse.");
		}

		dosFile.Flush();
		tosLog.Flush();
	}
	pi->ProgressEnd();
	return IMPEXP_SUCCESS;
}

int_32 DoExport(const char_x * szFileName, ExpInterface * pei, Interface * pi, BOOL bSuppressPrompts, DWORD dwOptions)
{
	CWin32App app;

	C2DDeviceGdip rsGdip;
	Set2DDevice(&rsGdip);

#ifdef _UNICODE
	int_32 iRet = ExportToVmp(szFileName, pei, pi, bSuppressPrompts, dwOptions);
#else
	char_16 szFileNameW[MAX_FILE_PATH] = {};
	ansitounicode(szFileName, -1, szFileNameW, MAX_FILE_PATH);
	int_32 iRet =  ExportToVmp(szFileNameW, pei, pi, bSuppressPrompts, dwOptions);
#endif
	Set2DDevice(nullptr);
	return iRet;
}
