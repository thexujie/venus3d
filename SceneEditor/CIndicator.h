#pragma once

#include "BaseInc.h"

class CIndicator : public C3DNode<I3DRenderObject>
{
public:
	CIndicator(I3DExplorer * pExplorer);
	~CIndicator();

	err_t Create();

	void Render(I3DContext * pContext, int_x iSubSet) const;
private:
	I3DBuffer * m_pvb;
	I3DEffect * m_pEffect;
	I3DMaterial * m_pMaterial;
};
