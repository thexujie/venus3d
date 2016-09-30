#pragma once

#include "BaseInc.h"

VENUS_BEG

template<typename OT>
int_x GetObjectRef(OT * ptr)
{
	if(!ptr)
		return 0;
	else
	{
		ptr->AddRef();
		return ptr->Release();
	}
}

VENUS_END
