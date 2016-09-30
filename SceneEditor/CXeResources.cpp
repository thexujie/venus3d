#include "stdafx.h"
#include "CXeResources.h"
#include "CXeTerrain.h"

CXeResources::CXeResources()
{

}

CXeResources::~CXeResources()
{
	for_each(m_ters, [](IXeTerrain *& pter) { SafeRelease(pter); });
}

err_t CXeResources::Load(const char_16 * szFile)
{
	err_t err = err_ok;
	xml_doc doc;
	err = doc.load_file(szFile);
	if(err)
		return err;

	const xml_node & node_items = doc(L"items");
	for(const xml_node & child : node_items.children)
	{
		if(child.name == L"terrain")
		{
			CXeTerrain * pter = new CXeTerrain();
			pter->Load(child[L"file"].value);
			m_ters.add(pter);
		}
	}
	return err_ok;
}


vector<IXeTerrain *> & CXeResources::GetTerrains()
{
	return m_ters;
}
