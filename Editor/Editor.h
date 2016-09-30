#pragma once

#include "BaseInc.h"

class VENUS_API IEditorWindow
{
public:
	virtual ~IEditorWindow() {}

	virtual void OnRun() = 0;
};

#include "CEditorApp.h"
#include "CEditorApp3D.h"
