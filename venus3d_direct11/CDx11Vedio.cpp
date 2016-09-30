#include "stdafx.h"
#include "CDx11Vedio.h"
#include "CDxgiAdapter.h"
#include "CDx11Buffer.h"
#include "CDx11Texture.h"
#include "CDx11Context.h"
#include "CDx11FrameBuffer.h"
#include "CDx11Pass.h"

VENUS_BEG

class CD3DInclude : public ID3DInclude
{
public:
	CD3DInclude(const char_16 * szFolder) : m_folder(szFolder) {}
	~CD3DInclude()
	{
		for_each(m_files, [](I3DFile * pFile) { SafeRelease(pFile); });
		m_files.clear();
	}

	HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID * ppData, UINT *pBytes)
	{
		char_16 szFileNameW[MAX_FILE_PATH] = {};
		char_16 szFileNameInclude[MAX_FILE_PATH] = {};
		ansitounicode(pFileName, -1, szFileNameW, MAX_FILE_PATH);
		textmakepath(szFileNameInclude, MAX_FILE_PATH, m_folder.buffer, -1, szFileNameW, -1);

		I3DFile * pFile = Get3DFileDriver()->GetFile(szFileNameInclude);
		if(!pFile)
			return E_FAIL;

		*ppData = (void *)pFile->GetBuffer();
		*pBytes = (uint_32)pFile->GetSize();
		m_files.add(pFile);
		return S_OK;
	}

	HRESULT __stdcall Close(LPCVOID pData)
	{
		return S_OK;
	}

	chbufferw<MAX_FILE_PATH> m_folder;
	vector<I3DFile *> m_files;
};

// CreateDevice
typedef HRESULT(__stdcall * D3D11CreateDeviceFunT)(
	IDXGIAdapter * pAdapter,
	D3D_DRIVER_TYPE DriverType,
	HMODULE Software,
	UINT Flags,
	const D3D_FEATURE_LEVEL * pFeatureLevels,
	UINT FeatureLevels,
	UINT SDKVersion,
	ID3D11Device ** ppDevice,
	D3D_FEATURE_LEVEL * pFeatureLevel,
	ID3D11DeviceContext ** ppImmediateContext);

CDx11Vedio::CDx11Vedio() :
	m_hResult(S_OK),
	m_hDx11Module(NULL), m_hDxCompilerModule(NULL),
	m_eFeature(Dx11Feature_11_1),
	m_pDevice(nullptr), m_pDxgiFactory(nullptr), m_pDxgiAdapter(nullptr)
{
}

CDx11Vedio::~CDx11Vedio()
{
	Destroy();
}

int_x CDx11Vedio::Create(VedioModeE eMode, I3DAdapter * pAdapter)
{
	Device2DTypeE eDevice2DType = Device2DTypeUnknown;
	I2DDevice * pRenderService = Get2DDevice();
	if(pRenderService)
		eDevice2DType = pRenderService->GetType();

	m_hDx11Module = LoadLibraryW(DX_D3D11_DLL);
	if(!m_hDx11Module)
	{
		log2(L"Can not load library %s!", DX_D3D11_DLL);
		return 1;
	}
	m_hDxCompilerModule = LoadLibraryW(DX_COMPILER_DLL);
	if(!m_hDxCompilerModule)
	{
		log2(L"Can not load library %s!", DX_COMPILER_DLL);
		return 1;
	}
	m_pfnDxCompile = (D3DXCompileFunT)::GetProcAddress(m_hDxCompilerModule, "D3DCompile");
	if(!m_pfnDxCompile)
	{
		log2(L"Can not find D3DCompile in your %s!", DX_COMPILER_DLL);
		return 1;
	}
	m_pfnDxReflect = (D3DXReflectFunT)::GetProcAddress(m_hDxCompilerModule, "D3DReflect");
	if(!m_pfnDxReflect)
	{
		log2(L"Can not find D3DReflect in your %s!", DX_COMPILER_DLL);
		return 1;
	}

	D3D11CreateDeviceFunT pfnCreate = (D3D11CreateDeviceFunT)GetProcAddress(m_hDx11Module, "D3D11CreateDevice");
	if(!pfnCreate)
	{
		log2(L"Can not find D3D11CreateDevice in your d3d11.dll!");
		return 1;
	}

	// ------ Device
	D3D_DRIVER_TYPE eDriverType = D3D_DRIVER_TYPE_NULL;
	switch(eMode)
	{
	case VedioModeDefalult:
		eDriverType = D3D_DRIVER_TYPE_UNKNOWN;
		break;
	case VedioModeNull:
		eDriverType = D3D_DRIVER_TYPE_NULL;
		break;
	case VedioModeHardware:
		eDriverType = D3D_DRIVER_TYPE_HARDWARE;
		break;
	case VedioModeReference:
		eDriverType = D3D_DRIVER_TYPE_REFERENCE;
		break;
	case VedioModeSoftware:
		eDriverType = D3D_DRIVER_TYPE_SOFTWARE;
		break;
	case VedioModeWrap:
		eDriverType = D3D_DRIVER_TYPE_WARP;
		break;
	default:
		break;
	}

	uint_32 uiDeviceFlags = /*D3D11_CREATE_DEVICE_SINGLETHREADED*/0;
#ifdef _DEBUG
	uiDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	if(eDevice2DType == Device2DTypeDirect2D)
		uiDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	D3D_FEATURE_LEVEL eFeatureLevel = D3D_FEATURE_LEVEL_11_1;
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	// Get feature level.
	//m_hResult = pfnCreate(pAdapter, eDriverType, NULL, uiDeviceFlags,
	//					  featureLevels, 7, D3D11_SDK_VERSION, nullptr, &eFeatureLevel, nullptr);

	CDxgiAdapter * pDxAdapter = (CDxgiAdapter *)pAdapter;
	ID3D11DeviceContext * pContext = nullptr;
	m_hResult = pfnCreate(pDxAdapter ? pDxAdapter->DxGetAdapter() : nullptr, eDriverType,
		NULL, uiDeviceFlags,
		featureLevels, (uint_32)arraysize(featureLevels), D3D11_SDK_VERSION, &m_pDevice, &eFeatureLevel, &pContext);
	SafeRelease(pContext);

	if(m_hResult)
	{
		log2(L"D3D11CreateDevice failed : 0x%x", m_hResult);
		return 1;
	}

	switch(eFeatureLevel)
	{
	case D3D_FEATURE_LEVEL_9_1: m_eFeature = Dx11Feature_9_1; break;
	case D3D_FEATURE_LEVEL_9_2: m_eFeature = Dx11Feature_9_2; break;
	case D3D_FEATURE_LEVEL_9_3: m_eFeature = Dx11Feature_9_3; break;
	case D3D_FEATURE_LEVEL_10_0: m_eFeature = Dx11Feature_10_0; break;
	case D3D_FEATURE_LEVEL_10_1: m_eFeature = Dx11Feature_10_1; break;
	case D3D_FEATURE_LEVEL_11_0: m_eFeature = Dx11Feature_11_0; break;
	case D3D_FEATURE_LEVEL_11_1: m_eFeature = Dx11Feature_11_1; break;
	default: m_eFeature = Dx11Feature_0; break;
		break;
	}

	if(!m_pDevice)
		return 1;

	//IDXGIDevice2 * pDXGIDevice2 = nullptr;
	//m_hResult = m_pDevice->QueryInterface(__uuidof(IDXGIDevice2), (void **)&pDXGIDevice2);

	IDXGIDevice * pDXGIDevice = nullptr;
	m_hResult = m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);

	IDXGIAdapter * pDXGIAdapter = nullptr;
	m_hResult = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&pDXGIAdapter);

	m_hResult = pDXGIAdapter->GetParent(IID_PPV_ARGS(&m_pDxgiFactory));

	//Assert(m_pFactory == pFactory);
	SafeRelease(pDXGIAdapter);
	SafeRelease(pDXGIDevice);

	return 0;
}

void CDx11Vedio::Destroy()
{
	SafeRelease(m_pDxgiFactory);
#ifdef _DEBUG
	int_x iRef = GetObjectRef(m_pDevice);
	if(iRef > 1)
	{
		ID3D11Debug * pDebug = nullptr;
		m_pDevice->QueryInterface(__uuidof(ID3D11Debug), (void **)&pDebug);
		if(pDebug)
		{
			pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
			SafeRelease(pDebug);
		}
	}
#endif
	try
	{
		SafeRelease(m_pDevice);
	}
	catch(_com_error & e)
	{
		log2(L"Error:%s", e.ErrorMessage());
	}
	FreeLibrary(m_hDx11Module);
	m_hDx11Module = NULL;
}

I3DContext * CDx11Vedio::CreateContext()
{
	if(!m_pDevice)
		return nullptr;
	ID3D11DeviceContext * pContext = nullptr;
	m_pDevice->GetImmediateContext(&pContext);
	CDx11Context * pDxContext = new CDx11Context(this, pContext);
	SafeRelease(pContext);
	return pDxContext;
}

I3DFrameBuffer * CDx11Vedio::CreateFrameBuffer()
{
	if(!m_pDevice)
		return nullptr;

	CDx11FrameBuffer * pFrameBuffer = new CDx11FrameBuffer(this);
	return pFrameBuffer;
}

I3DTexture * CDx11Vedio::CreateTexture()
{
	if(!m_pDevice)
		return nullptr;

	CDx11Texture * pTexture = new CDx11Texture(this);
	return pTexture;
}

I3DBuffer * CDx11Vedio::CreateBuffer()
{
	if(!m_pDevice)
		return nullptr;
	I3DBuffer * pBuffer = new CDx11Buffer(this);
	return pBuffer;
}

I3DPass * CDx11Vedio::CreatePass()
{
	if(!m_pDevice)
		return nullptr;

	CDx11Pass * pPass = new CDx11Pass(this);
	return pPass;
}

ID3D11Device * CDx11Vedio::Dx11GetDevice() const
{
	return m_pDevice;
}

Dx11FeatureE CDx11Vedio::Dx11GetFeature() const
{
	return m_eFeature;
}

HRESULT CDx11Vedio::DxCompileFromMemory(shader_e eType,
	const byte_t * pData, int_x iLength,
	const char_8 * szFileName, const char_8 * szFunction,
	uint_32 uiFlags1, uint_32 uiFlags2,
	ID3D10Blob ** ppShader, ID3D10Blob ** ppErrorMsgs,
	ID3DInclude * pInclude)
{
	if(!m_pfnDxCompile)
		return E_INVALIDARG;

	const char_8 * szProfile = DX11_SHADER_PROFILE[eType][m_eFeature];
	//pfnCompile = D3DCompile;
	log0(L"CDx11Vedio::DxCompileFromMemory \"%S\" with %S.", szFunction, szProfile);
	HRESULT hResult = S_OK;
	m_hResult = m_pfnDxCompile(pData, iLength, szFileName,
		nullptr, pInclude,
		szFunction, szProfile,
		uiFlags1, uiFlags2,
		ppShader, ppErrorMsgs);
	return m_hResult;
}

HRESULT CDx11Vedio::DxCompile(shader_e shader,
	const char_16 * szFileName, const char_16 * szFunction,
	uint_32 uiFlags1, uint_32 uiFlags2,
	ID3D10Blob ** ppShader, ID3D10Blob ** ppErrorMsgs)
{
	if(shader < 0 || shader >= shader_num || !szFileName || !szFunction)
		return E_INVALIDARG;

	char_8 szFileNameA[MAX_FILE_PATH] = {};
	char_8 szFunctionA[EFFECT_MAX_IDEITIFIER] = {};

	unicodetoansi(szFileName, -1, szFileNameA, MAX_FILE_PATH);
	unicodetoansi(szFunction, -1, szFunctionA, EFFECT_MAX_IDEITIFIER);

	char_16 szFolder[MAX_FILE_PATH] = {};
	textsplitpath_folder(szFileName, -1, szFolder, MAX_FILE_PATH);

	I3DFile * pFile = Get3DFileDriver()->GetFile(szFileName);
	if(!pFile)
		return err_failed;

	const char_8 * szProfile = DX11_SHADER_PROFILE[shader][m_eFeature];
	CD3DInclude d3dInclude(szFolder);

	//pfnCompile = D3DCompile;
	log0(L"CDx11Vedio::DxCompile %s \"%s\" with %S.", szFileName, szFunction, szProfile);
	HRESULT hResult = S_OK;
	m_hResult = m_pfnDxCompile(
		(const void *)pFile->GetBuffer(), pFile->GetSize(), szFileNameA,
		nullptr, &d3dInclude,
		szFunctionA, szProfile,
		uiFlags1, uiFlags2,
		ppShader, ppErrorMsgs);
	SafeRelease(pFile);

	return m_hResult;
}

HRESULT CDx11Vedio::DxReflect(const byte_t * pData, int_x iLength, const IID & iid, void ** ppInterface)
{
	if(!m_pfnDxReflect)
		return E_INVALIDARG;

	m_hResult = m_pfnDxReflect(pData, iLength, iid, ppInterface);
	return m_hResult;
}

VENUS_END
