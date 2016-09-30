#pragma once

#include "IXeInc.h"

class CXeResources : public ObjectT<IXeResources>
{
public:
	CXeResources();
	~CXeResources();

	err_t Load(const char_16 * szFile);


	vector<IXeTerrain *> & GetTerrains();
private:
	vector<IXeTerrain *> m_ters;
};
