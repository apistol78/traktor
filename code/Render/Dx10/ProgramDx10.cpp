#include "Render/Dx10/Platform.h"
#include "Render/Dx10/ProgramDx10.h"
#include "Render/Dx10/ProgramResourceDx10.h"
#include "Render/Dx10/ContextDx10.h"
#include "Render/Dx10/SimpleTextureDx10.h"
#include "Render/Dx10/CubeTextureDx10.h"
#include "Render/Dx10/RenderTargetDx10.h"
#include "Render/Dx10/HlslProgram.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramDx10", ProgramDx10, IProgram)

ProgramDx10* ProgramDx10::ms_activeProgram = 0;

ProgramDx10::ProgramDx10(ContextDx10* context)
:	m_context(context)
,	m_stencilReference(0)
,	m_d3dInputElementsHash(0)
,	m_parameterArrayDirty(false)
,	m_parameterResArrayDirty(false)
{
}

ProgramDx10::~ProgramDx10()
{
	destroy();
}

Ref< ProgramResourceDx10 > ProgramDx10::compile(const HlslProgram& hlslProgram)
{
	Ref< ProgramResourceDx10 > resource = new ProgramResourceDx10();

	ComRef< ID3D10Blob > d3dVertexShaderBlob;
	ComRef< ID3D10Blob > d3dPixelShaderBlob;
	ComRef< ID3D10Blob > d3dErrorMsgs;
	HRESULT hr;

	hr = D3D10CompileShader(
		wstombs(hlslProgram.getVertexShader()).c_str(),
		hlslProgram.getVertexShader().length(),
		"",
		NULL,
		NULL,
		"main",
		"vs_4_0",
		0,
		&resource->m_vertexShader.getAssign(),
		&d3dErrorMsgs.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Failed to compile vertex shader, hr = " << int32_t(hr) << Endl;
		if (d3dErrorMsgs)
			log::error << mbstows((LPCSTR)d3dErrorMsgs->GetBufferPointer()) << Endl;
		log::error << Endl;
		FormatMultipleLines(log::error, hlslProgram.getVertexShader());
		return 0;
	}

	hr = D3D10CompileShader(
		wstombs(hlslProgram.getPixelShader()).c_str(),
		hlslProgram.getPixelShader().length(),
		"",
		NULL,
		NULL,
		"main",
		"ps_4_0",
		0,
		&resource->m_pixelShader.getAssign(),
		&d3dErrorMsgs.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Failed to compile pixel shader, hr = " << int32_t(hr) << Endl;
		if (d3dErrorMsgs)
			log::error << mbstows((LPCSTR)d3dErrorMsgs->GetBufferPointer()) << Endl;
		log::error << Endl;
		FormatMultipleLines(log::error, hlslProgram.getPixelShader());
		return 0;
	}

	resource->m_d3dRasterizerDesc = hlslProgram.getD3DRasterizerDesc();
	resource->m_d3dDepthStencilDesc = hlslProgram.getD3DDepthStencilDesc();
	resource->m_d3dBlendDesc = hlslProgram.getD3DBlendDesc();
	resource->m_stencilReference = hlslProgram.getStencilReference();
	resource->m_d3dVertexSamplers = hlslProgram.getD3DVertexSamplers();
	resource->m_d3dPixelSamplers = hlslProgram.getD3DPixelSamplers();

	return resource;
}

bool ProgramDx10::create(ID3D10Device* d3dDevice, const ProgramResourceDx10* resource, float mipBias)
{
	ComRef< ID3D10Blob > d3dErrorMsgs;
	HRESULT hr;

	hr = d3dDevice->CreateVertexShader(
		(DWORD*)resource->m_vertexShader->GetBufferPointer(),
		resource->m_vertexShader->GetBufferSize(),
		&m_d3dVertexShader.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Failed to create vertex shader, hr = " << int32_t(hr) << Endl;
		return false;
	}

	hr = d3dDevice->CreatePixelShader(
		(DWORD*)resource->m_pixelShader->GetBufferPointer(),
		resource->m_pixelShader->GetBufferSize(),
		&m_d3dPixelShader.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Failed to create pixel shader, hr = " << int32_t(hr) << Endl;
		return false;
	}

	m_d3dVertexShaderBlob = resource->m_vertexShader;

	// Create states.
	if (!createState(
		d3dDevice,
		mipBias,
		resource->m_vertexShader,
		resource->m_d3dVertexSamplers,
		/* [out] */
		m_vertexState
	))
		return false;

	if (!createState(
		d3dDevice,
		mipBias,
		resource->m_pixelShader,
		resource->m_d3dPixelSamplers,
		/* [out] */
		m_pixelState
	))
		return false;

	// Create rasterizer states.
	d3dDevice->CreateRasterizerState(
		&resource->m_d3dRasterizerDesc,
		&m_d3dRasterizerState.getAssign()
	);
	d3dDevice->CreateDepthStencilState(
		&resource->m_d3dDepthStencilDesc,
		&m_d3dDepthStencilState.getAssign()
	);
	d3dDevice->CreateBlendState(
		&resource->m_d3dBlendDesc,
		&m_d3dBlendState.getAssign()
	);

	m_stencilReference = resource->m_stencilReference;

	return true;
}

void ProgramDx10::destroy()
{
	if (ms_activeProgram == this)
		ms_activeProgram = 0;

	if (!m_context)
		return;

	m_context->releaseComRef(m_d3dRasterizerState);
	m_context->releaseComRef(m_d3dDepthStencilState);
	m_context->releaseComRef(m_d3dBlendState);
	m_context->releaseComRef(m_d3dVertexShader);
	m_context->releaseComRef(m_d3dPixelShader);
	m_context->releaseComRef(m_vertexState.d3dConstantBuffer);
	m_context->releaseComRef(m_vertexState.d3dSamplerStates);
	m_context->releaseComRef(m_pixelState.d3dConstantBuffer);
	m_context->releaseComRef(m_pixelState.d3dSamplerStates);
	m_context->releaseComRef(m_d3dVertexShaderBlob);
	for (std::map< size_t, ComRef< ID3D10InputLayout > >::iterator i = m_d3dInputLayouts.begin(); i != m_d3dInputLayouts.end(); ++i)
		m_context->releaseComRef(i->second);
	m_d3dInputLayouts.clear();
	m_context->releaseComRef(m_d3dInputLayout);
	m_context->releaseComRef(m_parameterResArray);

	m_parameterMap.clear();
	m_parameterFloatArray.resize(0);
}

void ProgramDx10::setFloatParameter(handle_t handle, float param)
{
	setFloatArrayParameter(handle, &param, 1);
}

void ProgramDx10::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	std::map< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		std::memcpy(&m_parameterFloatArray[i->second], param, length * sizeof(float));
		m_parameterArrayDirty = true;
	}
}

void ProgramDx10::setVectorParameter(handle_t handle, const Vector4& param)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(&param), 4);
}

void ProgramDx10::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(param), length * 4);
}

void ProgramDx10::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(&param), 16);
}

void ProgramDx10::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(param), length * 16);
}

void ProgramDx10::setTextureParameter(handle_t handle, ITexture* texture)
{
	std::map< handle_t, uint32_t >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		ID3D10ShaderResourceView* d3dTextureResourceView = 0;

		if (is_a< SimpleTextureDx10 >(texture))
			d3dTextureResourceView = static_cast< SimpleTextureDx10* >(texture)->getD3D10TextureResourceView();
		else if (is_a< CubeTextureDx10 >(texture))
			d3dTextureResourceView = static_cast< CubeTextureDx10* >(texture)->getD3D10TextureResourceView();
		else if (is_a< RenderTargetDx10 >(texture))
			d3dTextureResourceView = static_cast< RenderTargetDx10* >(texture)->getD3D10TextureResourceView();
		else
			return;

		m_parameterResArray[i->second] = d3dTextureResourceView;
		m_parameterResArrayDirty = true;
	}
}

void ProgramDx10::setStencilReference(uint32_t stencilReference)
{
	m_stencilReference = stencilReference;
}

bool ProgramDx10::bind(
	ID3D10Device* d3dDevice,
	size_t d3dInputElementsHash,
	const std::vector< D3D10_INPUT_ELEMENT_DESC >& d3dInputElements
)
{
	static float blendFactors[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	HRESULT hr;

	// Set states.
	d3dDevice->RSSetState(m_d3dRasterizerState);
	d3dDevice->OMSetDepthStencilState(m_d3dDepthStencilState, m_stencilReference);
	d3dDevice->OMSetBlendState(m_d3dBlendState, blendFactors, 0xffffffff);

	// Update constant buffers.
	if (m_parameterArrayDirty || ms_activeProgram != this)
	{
		if (!updateStateConstants(m_vertexState))
			return false;
		if (!updateStateConstants(m_pixelState))
			return false;
		m_parameterArrayDirty = false;		
	}

	// Bind constant buffers.
	d3dDevice->VSSetConstantBuffers(0, 1, &m_vertexState.d3dConstantBuffer.get());
	d3dDevice->PSSetConstantBuffers(0, 1, &m_pixelState.d3dConstantBuffer.get());

	// Bind samplers.
	if (!m_vertexState.d3dSamplerStates.empty())
		d3dDevice->VSSetSamplers(
			0,
			UINT(m_vertexState.d3dSamplerStates.size()),
			m_vertexState.d3dSamplerStates.base()
		);

	if (!m_pixelState.d3dSamplerStates.empty())
		d3dDevice->PSSetSamplers(
			0,
			UINT(m_pixelState.d3dSamplerStates.size()),
			m_pixelState.d3dSamplerStates.base()
		);

	// Bind resource views.
	if (m_parameterResArrayDirty || ms_activeProgram != this)
	{
		for (std::vector< std::pair< UINT, uint32_t > >::const_iterator i = m_vertexState.resourceIndices.begin(); i != m_vertexState.resourceIndices.end(); ++i)
		{
			ID3D10ShaderResourceView* res = m_parameterResArray[i->second];
			d3dDevice->VSSetShaderResources(i->first, 1, &res);
		}

		for (std::vector< std::pair< UINT, uint32_t > >::const_iterator i = m_pixelState.resourceIndices.begin(); i != m_pixelState.resourceIndices.end(); ++i)
		{
			ID3D10ShaderResourceView* res = m_parameterResArray[i->second];
			d3dDevice->PSSetShaderResources(i->first, 1, &res);
		}

		m_parameterResArrayDirty = false;
	}

	// Bind shaders.
	d3dDevice->VSSetShader(m_d3dVertexShader);
	d3dDevice->PSSetShader(m_d3dPixelShader);

	// Remap input layout if it has changed since last use of this shader.
	if (m_d3dInputElementsHash != d3dInputElementsHash || ms_activeProgram != this)
	{
		d3dDevice->IASetInputLayout(NULL);

		std::map< size_t, ComRef< ID3D10InputLayout > >::iterator i = m_d3dInputLayouts.find(d3dInputElementsHash);
		if (i != m_d3dInputLayouts.end())
			m_d3dInputLayout = i->second;
		else
		{
			// Layout hasn't been mapped yet; create new input layout.
			hr = d3dDevice->CreateInputLayout(
				&d3dInputElements[0],
				UINT(d3dInputElements.size()),
				m_d3dVertexShaderBlob->GetBufferPointer(),
				m_d3dVertexShaderBlob->GetBufferSize(),
				&m_d3dInputLayout.getAssign()
			);
			if (FAILED(hr))
				return false;

			// Save layout for later binding.
			m_d3dInputLayouts[d3dInputElementsHash] = m_d3dInputLayout;
		}

		m_d3dInputElementsHash = d3dInputElementsHash;
	}

	d3dDevice->IASetInputLayout(m_d3dInputLayout);

	ms_activeProgram = this;
	return true;
}

bool ProgramDx10::createState(
	ID3D10Device* d3dDevice,
	float mipBias,
	ID3D10Blob* d3dShaderBlob,
	const std::map< std::wstring, D3D10_SAMPLER_DESC >& d3dSamplers,
	State& outState
)
{
	ComRef< ID3D10ShaderReflection > d3dShaderReflection;
	ID3D10ShaderReflectionConstantBuffer* d3dConstantBufferReflection;
	ID3D10ShaderReflectionVariable* d3dVariableReflection;
	ID3D10ShaderReflectionType* d3dTypeReflection;
	D3D10_SHADER_DESC dsd;
	D3D10_SHADER_BUFFER_DESC dsbd;
	D3D10_SHADER_VARIABLE_DESC dsvd;
	D3D10_SHADER_TYPE_DESC dstd;
	D3D10_SHADER_INPUT_BIND_DESC dsibd;
	D3D10_BUFFER_DESC dbd;
	HRESULT hr;

	hr = D3D10ReflectShader(
		d3dShaderBlob->GetBufferPointer(),
		d3dShaderBlob->GetBufferSize(),
		&d3dShaderReflection.getAssign()
	);
	if (FAILED(hr))
		return false;

	d3dShaderReflection->GetDesc(&dsd);

	// Scalar parameters.
	if (dsd.ConstantBuffers >= 1)
	{
		d3dConstantBufferReflection = d3dShaderReflection->GetConstantBufferByIndex(0);
		T_ASSERT (d3dConstantBufferReflection);

		d3dConstantBufferReflection->GetDesc(&dsbd);
		T_ASSERT ((dsbd.Size & 3) == 0);

		dbd.ByteWidth = dsbd.Size;
		dbd.Usage = D3D10_USAGE_DYNAMIC;
		dbd.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
		dbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		dbd.MiscFlags = 0;

		hr = d3dDevice->CreateBuffer(&dbd, NULL, &outState.d3dConstantBuffer.getAssign());
		if (FAILED(hr))
			return false;

		for (UINT i = 0; i < dsbd.Variables; ++i)
		{
			d3dVariableReflection = d3dConstantBufferReflection->GetVariableByIndex(i);
			T_ASSERT (d3dVariableReflection);

			d3dTypeReflection = d3dVariableReflection->GetType();
			T_ASSERT (d3dTypeReflection);

			d3dVariableReflection->GetDesc(&dsvd);
			T_ASSERT ((dsvd.StartOffset & 3) == 0);

			d3dTypeReflection->GetDesc(&dstd);
			T_ASSERT (dstd.Type == D3D10_SVT_FLOAT);

			std::map< handle_t, uint32_t >::iterator it = m_parameterMap.find(getParameterHandle(mbstows(dsvd.Name)));
			if (it == m_parameterMap.end())
			{
				uint32_t parameterOffset = uint32_t(m_parameterFloatArray.size());
				uint32_t parameterCount = dsvd.Size >> 2;

				m_parameterFloatArray.resize(parameterOffset + parameterCount);

				outState.parameterFloatOffsets.push_back(ParameterOffset(
					dsvd.StartOffset,
					parameterOffset,
					parameterCount
				));

				m_parameterMap.insert(std::make_pair(getParameterHandle(mbstows(dsvd.Name)), parameterOffset));
			}
			else
			{
				uint32_t parameterOffset = it->second;
				uint32_t parameterCount = dsvd.Size >> 2;

				outState.parameterFloatOffsets.push_back(ParameterOffset(
					dsvd.StartOffset,
					parameterOffset,
					parameterCount
				));
			}
		}
	}

	// Texture/sampler parameters.
	for (UINT i = 0; i < dsd.BoundResources; ++i)
	{
		d3dShaderReflection->GetResourceBindingDesc(i, &dsibd);
		if (dsibd.Type == D3D10_SIT_TEXTURE)
		{
			T_ASSERT (dsibd.BindCount == 1);

			std::map< handle_t, uint32_t >::iterator it = m_parameterMap.find(getParameterHandle(mbstows(dsibd.Name)));
			if (it == m_parameterMap.end())
			{
				uint32_t resourceIndex = uint32_t(m_parameterResArray.size());

				m_parameterResArray.resize(resourceIndex + 1);
				m_parameterMap.insert(std::make_pair(getParameterHandle(mbstows(dsibd.Name)), resourceIndex));

				outState.resourceIndices.push_back(std::make_pair(dsibd.BindPoint, resourceIndex));
			}
			else
			{
				outState.resourceIndices.push_back(std::make_pair(dsibd.BindPoint, it->second));
			}
		}
		else if (dsibd.Type == D3D10_SIT_SAMPLER)
		{
			std::map< std::wstring, D3D10_SAMPLER_DESC >::const_iterator it = d3dSamplers.find(mbstows(dsibd.Name));
			if (it == d3dSamplers.end())
				return false;

			D3D10_SAMPLER_DESC dsd = it->second;
			dsd.MipLODBias = mipBias;

			ID3D10SamplerState* d3dSamplerState;

			hr = d3dDevice->CreateSamplerState(&dsd, &d3dSamplerState);
			if (FAILED(hr))
				return false;

			outState.d3dSamplerStates.push_back(d3dSamplerState);
		}
	}

	return true;
}

bool ProgramDx10::updateStateConstants(State& state)
{
	if (!state.d3dConstantBuffer)
		return true;

	uint8_t* mapped;
	HRESULT hr = state.d3dConstantBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&mapped);
	if (FAILED(hr))
		return false;

	for (std::vector< ParameterOffset >::const_iterator i = state.parameterFloatOffsets.begin(); i != state.parameterFloatOffsets.end(); ++i)
	{
		std::memcpy(
			&mapped[i->constant],
			&m_parameterFloatArray[i->offset],
			i->count * sizeof(float)
		);
	}

	state.d3dConstantBuffer->Unmap();
	return true;
}

	}
}
