#pragma once

#include "IXeInc.h"

class CXeScene: public ObjectT<IXeScene>
{
public:
	CXeScene();
	~CXeScene();

	err_t Load(const char_16 * szFile);

private:
};
