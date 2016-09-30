#include "stdafx.h"
#include "CXeInspectorPanel.h"

static const oid_t OID_XeInspectorPanel = {L"xeinspectorpanel"};

CXeInspectorPanel::CXeInspectorPanel()
{
}


CXeInspectorPanel::~CXeInspectorPanel()
{
}

const oid_t & CXeInspectorPanel::GetOid() const
{
	return OID_XeInspectorPanel;
}
