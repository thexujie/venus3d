#include "stdafx.h"
#include "CXeTerrain.h"

CXeTerrain::CXeTerrain()
{

}

CXeTerrain::~CXeTerrain()
{

}

err_t CXeTerrain::Load(const char_16 * szFile)
{
	chbufferw<MAX_FILE_PATH> path;
	textformat(path.buffer, MAX_FILE_PATH, L"terrains/%s", szFile);
	fileinfo_t finfo(path.buffer);
	if(!finfo.exists)
		return err_no_file;

	err_t err = err_ok;
	xml_doc doc;
	err = doc.load_file(finfo.path);
	if(err)
		return err;

	m_file = /*finfo.path*/szFile;
	m_name = doc[L"name"];
	m_nheight = doc(L"nheight").text;
	m_color= doc(L"color").text;
	m_layer= doc(L"layer").text;
	m_detail= doc(L"detail").text;
	return err_ok;
}

textw CXeTerrain::GetFile() const
{
	return m_file;
}

textw CXeTerrain::GetName() const
{
	return m_name;
}
