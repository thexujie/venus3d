#pragma once

#include "BaseInc.h"

class CToolForm : public CForm
{
public:
	CToolForm();
	~CToolForm();

	void Load(const char_16 * szXmlFile);
};
