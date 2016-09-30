#include "stdafx.h"
#include "CToolForm.h"

CToolForm::CToolForm()
{
	SetFormType(FormTypePopup);
	SetBorderType(BorderType3Inactive);
}

CToolForm::~CToolForm()
{

}

void CToolForm::Load(const char_16 * szXmlFile)
{
	IUILoader::GetInstance()->Load(szXmlFile, this);
	OnLoaded();
}
