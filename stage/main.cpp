#include "stdafx.h"
#include "BaseInc.h"
#include "CEditorForm.h"

void Test()
{
	CEditorForm form;
	form.Create2DDevice(Device2DTypeDirect2D);

	CImage image;
	image.Load(L"lion.dds");

	form.LoadFromXml(L"stage/ui/stage.xml");
	form.Show(ShowModeNormal, HostInitPosCenterScreen);
	form.Initialize3D();
	GetApp()->Run(form.GetFormId(), AppRunDialog);
}

int _tmain(int argc, _TCHAR* argv[])
{
	SetCurrentDirectoryW(L"../bin/");
#ifdef _DEBUG
	_CrtMemState stateOld, stateNew, stateDiff;
	_CrtMemCheckpoint(&stateOld);
	Test();
	_CrtMemCheckpoint(&stateNew);
	if(_CrtMemDifference(&stateDiff, &stateOld, &stateNew))
		_CrtMemDumpAllObjectsSince(&stateDiff);
#else
	Test();
#endif 
	return 0;
}
