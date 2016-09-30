#pragma once

#include "BaseInc.h"

class CXeInspectorPanel : public CPanel
{
public:
	CXeInspectorPanel();
	~CXeInspectorPanel();
	const oid_t & GetOid() const;
};

