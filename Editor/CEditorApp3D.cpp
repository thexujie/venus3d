#include "stdafx.h"
#include "CEditorApp3D.h"

CEditorApp3D::CEditorApp3D()
{
	m_pFileDriver = CreateFileDriver_Direct11();
	m_pVedioDriver = CreateVedioDriver_Direct11();
}

CEditorApp3D::~CEditorApp3D()
{
	SafeRelease(m_pVedioDriver);
	SafeRelease(m_pFileDriver);
}

I3DFileDriver * CEditorApp3D::GetFileDriver() const
{
	return m_pFileDriver;
}

I3DVedioDriver * CEditorApp3D::GetVedioDriver() const
{
	return m_pVedioDriver;
}

I3DAudioDriver * CEditorApp3D::GetAudioDriver() const
{
	return nullptr;
}

I3DInputDriver * CEditorApp3D::GetInputDriver() const
{
	return nullptr;
}

I3DExplorer * CEditorApp3D::CreateExplorer(I3DVedio * pVedio)
{
	return new C3DExplorer(pVedio);
}

I3DScene * CEditorApp3D::CreateScene(I3DExplorer * pExplorer)
{
	return new C3DScene(pExplorer);
}
