#include "stdafx.h"
#include "CDx11Effect.h"
#include "CDx11Context.h"
#include "CDx11Texture.h"
#include "CDx11Buffer.h"

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

	HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
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

CDx11Effect::CDx11Effect(CDx11Vedio * pDevice) :
	m_pVedio(pDevice), m_pContext(nullptr), m_pInputLayout(nullptr),
	m_pShaderVS(nullptr), m_pShaderHS(nullptr), m_pShaderDS(nullptr), m_pShaderPS(nullptr),
	m_pBlendState(nullptr), m_pRasterizeState(nullptr), m_pDepthStencilState(nullptr),
	m_sampleMask(0xFFFFFFFF)
{
	SafeAddRef(m_pVedio);

	buffset8((void *)m_blobs, shader_num * sizeof(ID3DBlob *), 0);
	buffset8((void *)m_cbBuffers, shader_num * DX11_EFFECT_MAX_BUFFER_COUNT * sizeof(int_x), 0);
	buffset8((void *)m_hwbuffers, shader_num * DX11_EFFECT_MAX_BUFFER_COUNT * sizeof(ID3D11Buffer *), 0);
	buffset8((void *)m_membuffers, shader_num * DX11_EFFECT_MAX_BUFFER_COUNT * sizeof(uint_8 *), 0);
	buffset8((void *)m_resetBuffers, shader_num * DX11_EFFECT_MAX_BUFFER_COUNT * sizeof(bool), 0);

	buffset8((void *)m_samplerStates, shader_num * DX11_SAMPLER_COUNT * sizeof(SamplerStateT), 0);
	buffset8((void *)m_samplers, shader_num * DX11_SAMPLER_COUNT * sizeof(ID3D11SamplerState *), 0);
	buffset8((bool *)m_samplerExists, shader_num * DX11_SAMPLER_COUNT * sizeof(bool), 0);

	m_blendFactor = float4(1.0f, 1.0f, 1.0f, 1.0f);
}

CDx11Effect::~CDx11Effect()
{
	SafeRelease(m_pRasterizeState);
	SafeRelease(m_pBlendState);
	SafeRelease(m_pShaderVS);
	SafeRelease(m_pShaderHS);
	SafeRelease(m_pShaderDS);
	SafeRelease(m_pShaderPS);
	SafeRelease(m_pInputLayout);

	for(int_x ishader = 0; ishader < shader_num; ++ishader)
	{
		vector<ID3D11ShaderResourceView *> & srvs = m_srvs[ishader];
		for(int_x isrv = 0; isrv < srvs.size(); ++isrv)
			SafeRelease(srvs[isrv]);
		srvs.clear();

		for(int_x iSampler = 0; iSampler < DX11_SAMPLER_COUNT; ++iSampler)
		{
			SafeRelease(m_samplers[ishader][iSampler]);
		}

		for(int_x iBuffer = 0; iBuffer < DX11_EFFECT_MAX_BUFFER_COUNT; ++iBuffer)
		{
			SafeRelease(m_hwbuffers[ishader][iBuffer]);
		}
		SafeRelease(m_blobs[ishader]);
	}
	SafeRelease(m_pVedio);
}

I3DVedio * CDx11Effect::GetVedio() const
{
	return m_pVedio;
}

const char_16 * CDx11Effect::GetName() const
{
	return m_szName;
}

err_t CDx11Effect::SetName(const char_16 * szName, int_x iLength)
{
	textcpy(m_szName, EFFECT_MAX_IDEITIFIER, szName, iLength);
	return err_ok;
}

err_t CDx11Effect::SetInputElements(const InputElementT * pElements, int_x iCount)
{
	HRESULT hResult = S_OK;
	ID3D11Device * pDxDevice = m_pVedio->Dx11GetDevice();

	ID3DBlob * pBlobVS = m_blobs[shader_vs];
	if(!pDxDevice || !pBlobVS)
		return err_nullptr;

	int_x iLength = 0;
	int_x iOffset = 0;
	D3D11_INPUT_ELEMENT_DESC eleDescs[DX11_EFFECT_IA_MAX_ELEMENTS] = {};
	for(int_x cnt = 0; cnt < iCount; ++cnt)
	{
		const InputElementT & ele = pElements[cnt];
		D3D11_INPUT_ELEMENT_DESC & desc = eleDescs[cnt];

		desc.SemanticName = ele.desc.buffer;
		desc.SemanticIndex = 0;
		desc.Format = DxgiGetElementFormat(ele.format);
		desc.InputSlot = 0;
		desc.AlignedByteOffset = (uint_32)iOffset;
		desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		desc.InstanceDataStepRate = 0;
		iOffset += Dx11MapElementFormatSize(ele.format);
	}

	ID3D11InputLayout * pLayout = nullptr;
	hResult = pDxDevice->CreateInputLayout(eleDescs, (uint_32)iCount,
		pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), &pLayout);
	if(hResult == S_OK)
	{
		m_pInputLayout = pLayout;
		return err_ok;
	}
	else
		return err_nullptr;
}

err_t CDx11Effect::SetShader(I3DExplorer * pExplorer, shader_e shader, const char_16 * szFile, const char_16 * szFunction)
{
	if(shader <= shader_invalid || shader_num <= shader)
		return err_invalidarg;

	char_8 szFunctionA[EFFECT_MAX_IDEITIFIER] = {};
	unicodetoansi(szFunction, -1, szFunctionA, EFFECT_MAX_IDEITIFIER);

	if(!szFile || !szFunctionA[0])
		return err_invalidarg;

	if(!m_pVedio)
		return err_not_initialized;

	I3DFile * pFileData = pExplorer->LoadFile(szFile);
	if(!pFileData)
		return err_failed;

	ID3D10Blob * pError = nullptr;
	ID3DBlob *& pBlob = m_blobs[shader];
	uint_32 uiFlag1 = 0;
	uiFlag1 |= D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
	const char_8 * pszFileName = nullptr;
#ifdef _DEBUG
	uiFlag1 |= D3D10_SHADER_DEBUG;
	uiFlag1 |= D3D10_SHADER_SKIP_OPTIMIZATION;
	uiFlag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL0;

	char_8 szFileName[MAX_FILE_PATH] = {};
	unicodetoansi(szFile, -1, szFileName, MAX_FILE_PATH);
	pszFileName = szFileName;
#endif

	char_16 szFolder[MAX_FILE_PATH] = {};
	textsplitpath_folder(szFile, -1, szFolder, MAX_FILE_PATH);
	CD3DInclude d3dInclude(szFolder);
	HRESULT hResult = m_pVedio->DxCompileFromMemory(shader,
		pFileData->GetBuffer(), pFileData->GetSize(),
		pszFileName, szFunctionA,
		uiFlag1, 0,
		&pBlob, &pError, &d3dInclude);

	if(pError)
	{
		log1(L"Shader code compile error : \n%S", pError->GetBufferPointer());
		SafeRelease(pError);
	}

	if(!pBlob)
		return err_failed;

	ID3D11ShaderReflection * pReflect = nullptr;
	m_pVedio->DxReflect((const byte_t *)pBlob->GetBufferPointer(), (int_x)pBlob->GetBufferSize(),
		__uuidof(ID3D11ShaderReflection), (void **)&pReflect);

	SafeRelease(pFileData);

	if(!pReflect)
	{
		log2(L"Can not get ID3D11ShaderReflection interface.");
		return err_failed;
	}

	D3D11_SHADER_DESC shaderDesc = {};
	pReflect->GetDesc(&shaderDesc);

	vector<cbuffer_variable> & variables = m_variables[shader];
	int_x iBufferIndex = 0;
	int_x iSamplerIndex = 0;
	int_x iSlotIndex = 0;
	D3D11_SHADER_BUFFER_DESC cbufferDesc = {};
	D3D11_SHADER_INPUT_BIND_DESC bindDesc = {};
	D3D11_SHADER_VARIABLE_DESC variableDesc = {};
	D3D11_SHADER_TYPE_DESC typeDesc = {};
	D3D11_SIGNATURE_PARAMETER_DESC signDesc = {};
	for(uint_32 cnt = 0; cnt < shaderDesc.InputParameters; ++cnt)
	{
		pReflect->GetInputParameterDesc((uint_32)cnt, &signDesc);
		signDesc.Mask = 0;
	}

	int_x num_cbuffers = 0;
	int_x num_srvs = 0;

	for(uint_32 cnt = 0; cnt < shaderDesc.BoundResources; ++cnt)
	{
		pReflect->GetResourceBindingDesc(cnt, &bindDesc);
		iSlotIndex = (int_x)bindDesc.BindPoint;
		switch(bindDesc.Type)
		{
		case D3D_SIT_TEXTURE:
		{
			dx11_param_t & param = _TryAddParam(bindDesc.Name);
			param.type = eff_param_type_texture_2d;
			param.slots[shader] = (int_x)bindDesc.BindPoint;
			param.sizes[shader] = (int_x)bindDesc.BindCount;
			++num_srvs;
			break;
		}
		case D3D_SIT_CBUFFER:
		case D3D_SIT_TBUFFER:
			++num_cbuffers;
			break;
		default:
			break;
		}
	}

	m_srvs[shader].resize(num_srvs, nullptr);

	for(uint_32 cnt = 0; cnt < shaderDesc.BoundResources; ++cnt)
	{
		pReflect->GetResourceBindingDesc(cnt, &bindDesc);
		iSlotIndex = (int_x)bindDesc.BindPoint;
		if(bindDesc.Type == D3D_SIT_CBUFFER || bindDesc.Type == D3D_SIT_TBUFFER)
		{
			ID3D11ShaderReflectionConstantBuffer * pcbuffer = pReflect->GetConstantBufferByName(bindDesc.Name);
			if(!pcbuffer)
				continue;

			pcbuffer->GetDesc(&cbufferDesc);
			log(4, "%S(%s): cbuffer(r%d) %s size=%d bytes.", szFile, szFunctionA, bindDesc.BindPoint, cbufferDesc.Name, cbufferDesc.Size);
			for(uint_32 ivar = 0; ivar < cbufferDesc.Variables; ++ivar)
			{
				ID3D11ShaderReflectionVariable * pshdrefVariable = pcbuffer->GetVariableByIndex(ivar);
				if(!pshdrefVariable)
					continue;

				ID3D11ShaderReflectionType * pshdrefType = pshdrefVariable->GetType();
				if(!pshdrefType)
					continue;

				pshdrefVariable->GetDesc(&variableDesc);
				pshdrefType->GetDesc(&typeDesc);

				cbuffer_variable variable = {variableDesc.Name, {bindDesc.BindPoint, variableDesc.StartOffset, variableDesc.Size}};
				variables.add(variable);
				log0("  %s.%s size=%d bytes.", cbufferDesc.Name, variableDesc.Name, variableDesc.Size);

				dx11_param_t & param = _TryAddParam(variableDesc.Name);
				param.type = Dx11::Mapping(typeDesc.Class, typeDesc.Type, (int_32)typeDesc.Rows, (int_32)typeDesc.Columns);
				param.slots[shader] = (int_x)bindDesc.BindPoint;
				param.offsets[shader] = (int_x)variableDesc.StartOffset;
				param.sizes[shader] = (int_x)variableDesc.Size;

				SafeNull(pshdrefType);
				SafeNull(pshdrefVariable);
			}
			SafeNull(pcbuffer);

			if(0 <= iSlotIndex && iSlotIndex < DX11_EFFECT_MAX_BUFFER_COUNT)
			{
				m_cbBuffers[shader][iSlotIndex] = cbufferDesc.Size;
			}
		}
		else if(bindDesc.Type == D3D_SIT_SAMPLER)
		{
			if(0 <= iSlotIndex && iSlotIndex < DX11_SAMPLER_COUNT)
			{
				SamplerStateT & state = m_samplerStates[shader][iSlotIndex];
				bool & bExists = m_samplerExists[shader][iSlotIndex];

				state.FilterMode = fitermode_linear;
				state.AddressModeU = addressmode_wrap;
				state.AddressModeV = addressmode_wrap;
				state.AddressModeW = addressmode_wrap;
				state.Anisotropy = 1;
				state.ComparisonFun = compare_never;
				bExists = true;
			}
		}
		else
		{
		}
	}
	SafeRelease(pReflect);

#if 0
	D3D11_SHADER_VARIABLE_DESC variableDesc = {};
	for(uint_32 cnt = 0; /*cnt < shaderDesc.ConstantBuffers*/false; ++cnt)
	{
		ID3D11ShaderReflectionConstantBuffer * pReflectBuffer = pReflect->GetConstantBufferByIndex(cnt);
		if(!pReflectBuffer)
			continue;

		pReflectBuffer->GetDesc(&cbufferDesc);
		SafeNull(pReflectBuffer);

		ID3D11ShaderReflectionVariable * pReflectVariable = pReflect->GetVariableByName(cbufferDesc.Name);
		if(!pReflectVariable)
			continue;

		pReflectVariable->GetDesc(&variableDesc);
		SafeNull(pReflectVariable);

		if(cbufferDesc.Type == D3D_CT_CBUFFER || cbufferDesc.Type == D3D_CT_TBUFFER)
		{
			int_x iIndex = cnt;
			if(0 <= iIndex && iIndex < DX11_BUFFER_SLOT_COUNT)
			{
				m_bufferSizes[eShaderType][iIndex] = cbufferDesc.Size;
			}
		}
		else if(cbufferDesc.Type == D3D_SVT_SAMPLER)
		{
			if(pReflectVariable)
			{

				if(0 <= variableDesc.StartSampler && variableDesc.StartSampler < DX11_SAMPLERS_COUNT)
				{
					SamplerStateT & state = m_samplerStates[ShaderTypePixel][variableDesc.StartSampler];
					state.FilterMode = FilterModePoint;
					state.AddressModeU = AddressModeClamp;
					state.AddressModeV = AddressModeClamp;
					state.AddressModeW = AddressModeClamp;
					state.Anisotropy = 1;
					state.ComparisionFun = ComparisionFunNever;
				}
			}
		}
		else
		{

		}
	}
#endif
	return hResult;
}

err_t CDx11Effect::SetSamplerState(shader_e eType, int_x iIndex, const SamplerStateT & state)
{
	if(eType <= shader_invalid || shader_num <= eType)
		return 1;

	if(iIndex < 0 || DX11_SAMPLER_COUNT <= iIndex)
		return 1;

	m_samplerStates[eType][iIndex] = state;
	SafeRelease(m_samplers[eType][iIndex]);
	return 0;
}

err_t CDx11Effect::SetPipeLineState(const PipelineStateT & state)
{
	m_state = state;
	return err_ok;
}

err_t CDx11Effect::RenderBeg(I3DContext * pContext)
{
	if(pContext == m_pContext)
		return err_ok;

	SafeRelease(m_pContext);
	m_pContext = pContext;
	SafeAddRef(m_pContext);
	return err_ok;
}

err_t CDx11Effect::Render(I3DContext * pContext)
{
	HRESULT hResult = S_OK;
	ID3D11DeviceContext * pDxContext = ((CDx11Context *)m_pContext)->Dx11GetContext();
	ID3D11Device * pDxDevice = m_pVedio->Dx11GetDevice();

	ID3DBlob * pBlobVS = m_blobs[shader_vs];
	ID3DBlob * pBlobHS = m_blobs[shader_hs];
	ID3DBlob * pBlobDS = m_blobs[shader_ds];
	ID3DBlob * pBlobPS = m_blobs[shader_ps];

	if(pBlobVS && !m_pShaderVS)
	{
		hResult = pDxDevice->CreateVertexShader(pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), nullptr, &m_pShaderVS);
	}

	if(pBlobHS && !m_pShaderHS)
	{
		hResult = pDxDevice->CreateHullShader(pBlobHS->GetBufferPointer(), pBlobHS->GetBufferSize(), nullptr, &m_pShaderHS);
	}

	if(pBlobDS && !m_pShaderDS)
	{
		hResult = pDxDevice->CreateDomainShader(pBlobDS->GetBufferPointer(), pBlobDS->GetBufferSize(), nullptr, &m_pShaderDS);
	}

	if(pBlobPS && !m_pShaderPS)
	{
		hResult = pDxDevice->CreatePixelShader(pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), nullptr, &m_pShaderPS);
	}

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.MinLOD = F32_MIN;
	samplerDesc.MaxLOD = F32_MIN;
	samplerDesc.MipLODBias = 0.0f;
	for(int_x iShader = 0; iShader < shader_num; ++iShader)
	{
		for(int_x iSampler = 0; iSampler < DX11_SAMPLER_COUNT; ++iSampler)
		{
			const SamplerStateT & state = m_samplerStates[iShader][iSampler];
			ID3D11SamplerState *& pSampler = m_samplers[iShader][iSampler];
			bool bExists = m_samplerExists[iShader][iSampler];
			if(bExists && !pSampler)
			{
				samplerDesc.Filter = Dx11MapMinMagMipFilterMode(state.FilterMode);
				samplerDesc.AddressU = Dx11MapTextureAddressMode(state.AddressModeU);
				samplerDesc.AddressV = Dx11MapTextureAddressMode(state.AddressModeV);
				samplerDesc.AddressW = Dx11MapTextureAddressMode(state.AddressModeW);
				samplerDesc.MaxAnisotropy = (uint_32)state.Anisotropy;
				samplerDesc.ComparisonFunc = Dx11MapComparisionFun(state.ComparisonFun);
				hResult = pDxDevice->CreateSamplerState(&samplerDesc, &pSampler);
			}
		}
	}

	if(!m_pRasterizeState)
	{
		D3D11_RASTERIZER_DESC rasDesc = {};
		rasDesc.AntialiasedLineEnable = m_state.AntiAliase;
		rasDesc.CullMode = Dx11MapCullMode(m_state.CullMode);
		// TODO:Æ«ÒÆ
		rasDesc.DepthBias = (int_32)m_state.DepthBias;
		rasDesc.DepthBiasClamp = (float_32)rasDesc.DepthBias;
		rasDesc.DepthClipEnable = m_state.DepthClip;
		rasDesc.FillMode = Dx11MapFillMode(m_state.FillMode);
		rasDesc.FrontCounterClockwise = m_state.ClockWise;
		rasDesc.MultisampleEnable = m_state.MultiSample;
		rasDesc.ScissorEnable = m_state.Scissor;
		rasDesc.SlopeScaledDepthBias = 0.0001f;
		pDxDevice->CreateRasterizerState(&rasDesc, &m_pRasterizeState);
	}

	if(!m_pDepthStencilState)
	{
		D3D11_DEPTH_STENCIL_DESC dsDesc = {};
		dsDesc.DepthEnable = m_state.depth;
		dsDesc.DepthWriteMask = m_state.depth_write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = Dx11MapComparisionFun(m_state.depth_cmp);
		dsDesc.StencilEnable = false;
		pDxDevice->CreateDepthStencilState(&dsDesc, &m_pDepthStencilState);
	}

	if(!m_pBlendState)
	{
		D3D11_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = m_state.AlphaToCoverage;
		blendDesc.RenderTarget[0].BlendEnable = m_state.BlendEnable;
		blendDesc.RenderTarget[0].SrcBlend = Dx11MapBlend(m_state.SrcColor);
		blendDesc.RenderTarget[0].DestBlend = Dx11MapBlend(m_state.DstColor);
		blendDesc.RenderTarget[0].BlendOp = Dx11MapBlendOp(m_state.BlendColor);

		blendDesc.RenderTarget[0].SrcBlendAlpha = Dx11MapBlend(m_state.SrcAlpha);
		blendDesc.RenderTarget[0].DestBlendAlpha = Dx11MapBlend(m_state.DstAlpha);
		blendDesc.RenderTarget[0].BlendOpAlpha = Dx11MapBlendOp(m_state.BlendAlpha);
		blendDesc.RenderTarget[0].RenderTargetWriteMask = Dx11MapRenderTargetWriteMask(m_state.WriteMask);
		hResult = pDxDevice->CreateBlendState(&blendDesc, &m_pBlendState);
	}

	//D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	//hResult = pDxDevice->CreateDepthStencilState()

	for(int_x ishader = 0; ishader < shader_num; ++ishader)
	{
		for(int_x islot = 0; islot < DX11_EFFECT_MAX_BUFFER_COUNT; ++islot)
		{
			bool & reset = m_resetBuffers[ishader][islot];
			int_x iTotal = m_cbBuffers[ishader][islot];
			ID3D11Buffer *& hwbuffer = m_hwbuffers[ishader][islot];
			const uint_8 * membuffer = m_membuffers[ishader][islot];

			if(!reset)
				continue;
			reset = false;

			// create hw buffer now.
			if(!hwbuffer)
			{
				ID3D11Device * pDxDevice = ((CDx11Vedio *)m_pVedio)->Dx11GetDevice();
				D3D11_BUFFER_DESC bufferDesc = {};
				bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
				bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				bufferDesc.MiscFlags = 0;
				bufferDesc.StructureByteStride = 0;
				bufferDesc.ByteWidth = (UINT)(iTotal);
				hResult = pDxDevice->CreateBuffer(&bufferDesc, nullptr, &hwbuffer);
			}

			D3D11_MAPPED_SUBRESOURCE mapResource = {};
			hResult = pDxContext->Map(hwbuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapResource);
			buffcpy8((byte_t *)mapResource.pData, iTotal, membuffer, iTotal);
			pDxContext->Unmap(hwbuffer, 0);
		}
	}

	pDxContext->IASetInputLayout(m_pInputLayout);

	pDxContext->VSSetShader(m_pShaderVS, nullptr, 0);
	pDxContext->HSSetShader(m_pShaderHS, nullptr, 0);
	pDxContext->DSSetShader(m_pShaderDS, nullptr, 0);
	pDxContext->PSSetShader(m_pShaderPS, nullptr, 0);

	pDxContext->VSSetConstantBuffers(0, DX11_EFFECT_MAX_BUFFER_COUNT, m_hwbuffers[shader_vs]);
	pDxContext->PSSetConstantBuffers(0, DX11_EFFECT_MAX_BUFFER_COUNT, m_hwbuffers[shader_ps]);
	pDxContext->GSSetConstantBuffers(0, DX11_EFFECT_MAX_BUFFER_COUNT, m_hwbuffers[shader_gs]);
	pDxContext->HSSetConstantBuffers(0, DX11_EFFECT_MAX_BUFFER_COUNT, m_hwbuffers[shader_hs]);
	pDxContext->DSSetConstantBuffers(0, DX11_EFFECT_MAX_BUFFER_COUNT, m_hwbuffers[shader_ds]);
	pDxContext->CSSetConstantBuffers(0, DX11_EFFECT_MAX_BUFFER_COUNT, m_hwbuffers[shader_cs]);

	pDxContext->VSSetShaderResources(0, (uint_32)m_srvs[shader_vs].size(), m_srvs[shader_vs].buffer());
	pDxContext->PSSetShaderResources(0, (uint_32)m_srvs[shader_ps].size(), m_srvs[shader_ps].buffer());
	pDxContext->GSSetShaderResources(0, (uint_32)m_srvs[shader_gs].size(), m_srvs[shader_gs].buffer());
	pDxContext->HSSetShaderResources(0, (uint_32)m_srvs[shader_hs].size(), m_srvs[shader_hs].buffer());
	pDxContext->DSSetShaderResources(0, (uint_32)m_srvs[shader_ds].size(), m_srvs[shader_ds].buffer());
	pDxContext->CSSetShaderResources(0, (uint_32)m_srvs[shader_cs].size(), m_srvs[shader_cs].buffer());

	pDxContext->VSSetSamplers(0, DX11_SAMPLER_COUNT, m_samplers[shader_vs]);
	pDxContext->HSSetSamplers(0, DX11_SAMPLER_COUNT, m_samplers[shader_hs]);
	pDxContext->DSSetSamplers(0, DX11_SAMPLER_COUNT, m_samplers[shader_ds]);
	pDxContext->PSSetSamplers(0, DX11_SAMPLER_COUNT, m_samplers[shader_ps]);

	pDxContext->RSSetState(m_pRasterizeState);
	pDxContext->OMSetDepthStencilState(m_pDepthStencilState, 0);
	pDxContext->OMSetBlendState(m_pBlendState, m_blendFactor, m_sampleMask);

	//pDxContext->OMSetDepthStencilState()
	return err_ok;
}

err_t CDx11Effect::RenderEnd(I3DContext * pContext)
{
	return err_ok;
}

eff_paramid_t CDx11Effect::FindParam(const char_16 * name) const
{
	for(int_x cnt = 0; cnt < m_params.size(); ++cnt)
	{
		const dx11_param_t & param = m_params[cnt];
		if(param.name == name)
		{
			eff_paramid_t id(cnt);
#ifdef _DEBUG
			id.name = name;
#endif
			return id;
		}
	}

	eff_paramid_t id(-1);
#ifdef _DEBUG
	id.name = name;
#endif
	return id;
}

err_t CDx11Effect::SetTexture(eff_paramid_t id, I3DTexture * pTexture)
{
	const dx11_param_t & dxparam = m_params[id];
	if(dxparam.type != eff_param_type_texture_2d)
		return err_invalidcall;
	
	CDx11Texture * pdx11Texture = dynamic_cast<CDx11Texture *>(pTexture);
	ID3D11ShaderResourceView * psrv = pdx11Texture ? pdx11Texture->Dx11GetSRV() : nullptr;
	for(int_x cnt = 0; cnt < shader_num; ++cnt)
	{
		int_x slot = dxparam.slots[cnt];
		int_x size = dxparam.sizes[cnt];
		if(slot >= 0 && size > 0)
		{
			SafeRelease(m_srvs[cnt][slot]);
			m_srvs[cnt][slot] = psrv;
			SafeAddRef(m_srvs[cnt][slot]);
		}
	}
	return err_ok;
}

err_t CDx11Effect::SetTexture(const char_16 * name, I3DTexture * pTexture)
{
	return SetTexture(FindParam(name), pTexture);
}

err_t CDx11Effect::SetBuffer(eff_paramid_t id, I3DBuffer * pBuffer)
{
	const dx11_param_t & dxparam = m_params[id];
	if(dxparam.type != eff_param_type_buffer)
		return err_invalidcall;

	CDx11Buffer * pdx11Buffer = dynamic_cast<CDx11Buffer *>(pBuffer);
	ID3D11ShaderResourceView * psrv = pdx11Buffer ? pdx11Buffer->Dx11GetSRV() : nullptr;
	for(int_x cnt = 0; cnt < shader_num; ++cnt)
	{
		int_x slot = dxparam.slots[cnt];
		if(slot >= 0)
		{
			SafeRelease(m_srvs[cnt][slot]);
			m_srvs[cnt][slot] = psrv;
			SafeAddRef(m_srvs[cnt][slot]);
		}
	}
	return err_ok;
}

err_t CDx11Effect::SetBuffer(const char_16 * name, I3DBuffer * pBuffer)
{
	return SetBuffer(FindParam(name), pBuffer);
}

err_t CDx11Effect::SetCBuffer(eff_paramid_t id, const void * data, int_x size)
{
	const dx11_param_t & dxparam = m_params[id];
	for(int_x ishader = 0; ishader < shader_num; ++ishader)
	{
		int_x slot = dxparam.slots[ishader];
		int_x offset = dxparam.offsets[ishader];
		int_x size = dxparam.sizes[ishader];

		if(slot >= 0 && offset >= 0 && size > 0)
		{
			int_x total = m_cbBuffers[ishader][slot];
			if(offset + size > total)
				continue;

			uint_8 *& staging = m_membuffers[ishader][slot];
			if(!staging)
				staging = new uint_8[total];

			bool & reset = m_resetBuffers[ishader][slot];
			buffcpy8(staging + offset, data, size);
			reset = true;
		}
	}
	return err_ok;
}

err_t CDx11Effect::SetCBuffer(const char_16 * name, const void * data, int_x size)
{
	return SetCBuffer(FindParam(name), data, size);
}

dx11_param_t & CDx11Effect::_TryAddParam(const char_8 * szName)
{
	char_16 szNameW[EFFECT_MAX_IDEITIFIER];
	ansitounicode(szName, -1, szNameW, EFFECT_MAX_IDEITIFIER);
	for(int_x cnt = 0; cnt < m_params.size(); ++cnt)
	{
		dx11_param_t & param = m_params[cnt];
		if(param.name.equal(szNameW))
			return param;
	}

	dx11_param_t & param_new = m_params.add();
	param_new.name = szNameW;
	arrayfill(param_new.slots, (int_x)0);
	arrayfill(param_new.offsets, (int_x)0);
	arrayfill(param_new.sizes, (int_x)0);
	return param_new;
}

VENUS_END
