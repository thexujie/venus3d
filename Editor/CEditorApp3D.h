#pragma once

#include "BaseInc.h"

class VENUS_API CEditorApp3D : public ObjectT<IApp3D>
{
public:
	CEditorApp3D();
	~CEditorApp3D();

	I3DFileDriver * GetFileDriver() const;
	I3DVedioDriver * GetVedioDriver() const;
	I3DAudioDriver * GetAudioDriver() const;
	I3DInputDriver * GetInputDriver() const;

	I3DExplorer * CreateExplorer(I3DVedio * pVedio);
	I3DScene * CreateScene(I3DExplorer * pExplorer);

protected:
	I3DFileDriver * m_pFileDriver;
	I3DVedioDriver * m_pVedioDriver;
};
