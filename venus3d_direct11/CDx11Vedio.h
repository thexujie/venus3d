#pragma once

#include "Dx11Inc.h"

VENUS_BEG

// Compile Shader D3DCompile
typedef HRESULT(WINAPI * D3DXCompileFunT)(
	const void * pData, int_x iLength, const char_8 * szFileName,
	const D3D_SHADER_MACRO * pDefines, ID3DInclude * pInclude,
	const char_8 *szFunction, const char_8 * szProfile,
	uint_32 uiFlags1, uint_32 uiFlags2,
	ID3DBlob ** ppShader, ID3DBlob ** ppErrorMsgs);

// D3DReflect
typedef HRESULT(WINAPI * D3DXReflectFunT)(const byte_t * pData, int_x iLength, const IID & iid, void ** ppInterface);

const char_16 DX_D3D11_DLL[] = L"d3d11.dll";
const char_16 DX_COMPILER_DLL[] = L"d3dcompiler_47.dll";

class CDx11Vedio : public ObjectT<I3DVedio>
{
public:
	CDx11Vedio();
	~CDx11Vedio();

	int_x Create(VedioModeE eMode, I3DAdapter * pAdapter);
	void Destroy();

	I3DContext * CreateContext();
	I3DFrameBuffer * CreateFrameBuffer();
	I3DTexture * CreateTexture();
	I3DBuffer * CreateBuffer();
	I3DPass * CreatePass();

public:
	IDXGIFactory * Dx11GetFactory() const { return m_pDxgiFactory; }
	ID3D11Device * Dx11GetDevice() const;
	Dx11FeatureE Dx11GetFeature() const;
	HRESULT DxCompileFromMemory(shader_e eType,
		const byte_t * pData, int_x iLength,
		const char_8 * szFileName, const char_8 * szFunction,
		uint_32 uiFlags1, uint_32 uiFlags2,
		ID3D10Blob ** ppShader, ID3D10Blob ** ppErrorMsgs,
		ID3DInclude * pInclude);
	HRESULT DxCompile(shader_e eType,
		const char_16 * szFileName, const char_16 * szFunction,
		uint_32 uiFlags1, uint_32 uiFlags2,
		ID3D10Blob ** ppShader, ID3D10Blob ** ppErrorMsgs);
	HRESULT DxReflect(const byte_t * pData, int_x iLength, const IID & iid, void ** ppInterface);

private:
	mutable HRESULT m_hResult;
	HMODULE m_hDx11Module;
	HMODULE m_hDxCompilerModule;

	D3DXCompileFunT m_pfnDxCompile;
	D3DXReflectFunT m_pfnDxReflect;

	IDXGIAdapter * m_pDxgiAdapter;

	Dx11FeatureE m_eFeature;
	ID3D11Device * m_pDevice;
	IDXGIFactory * m_pDxgiFactory;
};

VENUS_END
