#pragma once

#include "IXeInc.h"

class CXeSolution : public ObjectT<IXeSolution>
{
public:
	CXeSolution();
	~CXeSolution();

	err_t Load(const char_16 * szFile);

	IXeResources * GetResources();
private:
	IXeResources * m_pxres;
	vector<IXeScene *> m_scenes;
};
