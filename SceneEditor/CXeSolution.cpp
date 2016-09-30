#include "stdafx.h"
#include "CXeSolution.h"
#include "CXeResources.h"
#include "CXeScene.h"

CXeSolution::CXeSolution():m_pxres(nullptr)
{

}

CXeSolution::~CXeSolution()
{
	SafeRelease(m_pxres);
	for_each(m_scenes, [](IXeScene *& pscene) { SafeRelease(pscene); });
}

err_t CXeSolution::Load(const char_16 * szFile)
{
	err_t err = err_ok;
	xml_doc doc;
	err = doc.load_file(szFile);
	if(err)
		return err;

	CXeResources * pxres = new CXeResources();
	pxres->Load(doc(L"resources").text);
	m_pxres = pxres;
	return err_ok;
}

IXeResources * CXeSolution::GetResources()
{
	return m_pxres;
}
