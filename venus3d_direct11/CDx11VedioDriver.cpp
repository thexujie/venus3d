#include "stdafx.h"
#include "CDx11VedioDriver.h"

#include "CDx11Vedio.h"
#include "CDxgiAdapter.h"

VENUS_BEG

CDx11VedioDriver::CDx11VedioDriver():
m_hDxgi(NULL), m_pFactory(nullptr)
{
}

CDx11VedioDriver::~CDx11VedioDriver()
{
	SafeRelease(m_pFactory);
	if(m_hDxgi)
	{
		::FreeLibrary(m_hDxgi);
		m_hDxgi = NULL;
	}
}

err_t CDx11VedioDriver::Initialize()
{
	if(!m_hDxgi)
	{
		m_hDxgi = ::LoadLibraryW(L"dxgi.dll");
		if(!m_hDxgi)
		{
			log2(L"Can not load library dxgi.dll!");
			return err_no_file;
		}
	}

	if(!m_pFactory)
	{
		typedef HRESULT(__stdcall * CreateDXGIFactory_FunT)(const IID & riid, void ** ppFactory);
		CreateDXGIFactory_FunT pfnCreate = (CreateDXGIFactory_FunT)GetProcAddress(m_hDxgi, "CreateDXGIFactory");
		if(pfnCreate)
		{
			HRESULT hResult = pfnCreate(__uuidof(IDXGIFactory), (void **)&m_pFactory);
			if(!m_pFactory)
			{
				log2(L"Can not Create IDXGIFactory!");
				return err_failed;
			}
		}
	}
	return err_ok;
}

I3DAdapter * CDx11VedioDriver::EnumAdapter(int_x iIndex)
{
	if(!m_pFactory)
		return nullptr;

	IDXGIAdapter * pAdapter = nullptr;
	HRESULT hResult = m_pFactory->EnumAdapters((uint_32)iIndex, &pAdapter);
	if(hResult == DXGI_ERROR_NOT_FOUND)
		return nullptr;
	else
		return new CDxgiAdapter(pAdapter);
}

I3DVedio * CDx11VedioDriver::CreateVedio(VedioModeE eMode, I3DAdapter * pAdapter)
{
	CDx11Vedio * pDevice = new CDx11Vedio();
	int_x iRet = pDevice->Create(eMode, pAdapter);
	if(iRet)
		return nullptr;
	else
		return  pDevice;
}

VENUS_END
