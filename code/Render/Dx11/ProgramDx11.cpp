#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/TString.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/CubeTextureDx11.h"
#include "Render/Dx11/HlslProgram.h"
#include "Render/Dx11/ProgramDx11.h"
#include "Render/Dx11/ProgramResourceDx11.h"
#include "Render/Dx11/RenderTargetDx11.h"
#include "Render/Dx11/ResourceCache.h"
#include "Render/Dx11/SimpleTextureDx11.h"
#include "Render/Dx11/StateCache.h"
#include "Render/Dx11/VolumeTextureDx11.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

render::handle_t s_targetSizeHandle = 0;

bool storeIfNotEqual(const float* source, int length, float* dest)
{
	for (int i = 0; i < length; ++i)
	{
		if (abs(dest[i] - source[i]) > FUZZY_EPSILON)
		{
			for (; i < length; ++i)
				dest[i] = source[i];
			return true;
		}
	}
	return false;
}

bool storeIfNotEqual(const Vector4* source, int length, float* dest)
{
	for (int i = 0; i < length; ++i)
	{
		if (!compareFuzzyEqual(Vector4::loadAligned(&dest[i * 4]), source[i]))
		{
			for (; i < length; ++i)
				source[i].storeAligned(&dest[i * 4]);
			return true;
		}
	}
	return false;	
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramDx11", ProgramDx11, IProgram)

ProgramDx11* ProgramDx11::ms_activeProgram = 0;

ProgramDx11::ProgramDx11(ContextDx11* context)
:	m_context(context)
,	m_stencilReference(0)
,	m_d3dVertexShaderHash(0)
,	m_parameterTextureArrayDirty(false)
#if defined(_DEBUG)
,	m_bindCount(0)
#endif
{
	s_targetSizeHandle = getParameterHandle(L"_dx11_targetSize");
}

ProgramDx11::~ProgramDx11()
{
	destroy();
}

bool ProgramDx11::create(
	ID3D11Device* d3dDevice,
	ResourceCache& resourceCache,
	const ProgramResourceDx11* resource
)
{
	// Get shaders; reuse existing if already created.
	m_d3dVertexShader = resourceCache.getVertexShader(resource->m_vertexShader, resource->m_vertexShaderHash);
	if (!m_d3dVertexShader)
	{
		log::error << L"Failed to create vertex shader" << Endl;
		return false;
	}

	m_d3dPixelShader = resourceCache.getPixelShader(resource->m_pixelShader, resource->m_pixelShaderHash);
	if (!m_d3dPixelShader)
	{
		log::error << L"Failed to create pixel shader" << Endl;
		return false;
	}

	m_d3dVertexShaderBlob = resource->m_vertexShader;
	m_d3dVertexShaderHash = resource->m_vertexShaderHash;

	// Create states.
	if (!createState(
		d3dDevice,
		resourceCache,
		0,
		resource->m_vertexShader,
		resource->m_d3dVertexSamplers,
		/* [out] */
		m_vertexState
	))
		return false;

	if (!createState(
		d3dDevice,
		resourceCache,
		1,
		resource->m_pixelShader,
		resource->m_d3dPixelSamplers,
		/* [out] */
		m_pixelState
	))
		return false;

	// Create state objects.
	m_d3dRasterizerState = resourceCache.getRasterizerState(resource->m_d3dRasterizerDesc);
	m_d3dDepthStencilState = resourceCache.getDepthStencilState(resource->m_d3dDepthStencilDesc);
	m_d3dBlendState = resourceCache.getBlendState(resource->m_d3dBlendDesc);

	m_stencilReference = resource->m_stencilReference;
	return true;
}

void ProgramDx11::destroy()
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
	for (uint32_t i = 0; i < sizeof_array(m_vertexState.cbuffer); ++i)
		m_context->releaseComRef(m_vertexState.cbuffer[i].d3dBuffer);
	m_context->releaseComRef(m_vertexState.d3dSamplerStates);
	for (uint32_t i = 0; i < sizeof_array(m_pixelState.cbuffer); ++i)
		m_context->releaseComRef(m_pixelState.cbuffer[i].d3dBuffer);
	m_context->releaseComRef(m_pixelState.d3dSamplerStates);
	m_context->releaseComRef(m_d3dVertexShaderBlob);

	m_parameterMap.clear();
	m_parameterFloatArray.resize(0);
	m_parameterTextureArray.resize(0);
}

void ProgramDx11::setFloatParameter(handle_t handle, float param)
{
	setFloatArrayParameter(handle, &param, 1);
}

void ProgramDx11::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	SmallMap< handle_t, ParameterMap >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		if (storeIfNotEqual(param, length, &m_parameterFloatArray[i->second.offset]))
		{
			if (i->second.cbuffer[0])
				i->second.cbuffer[0]->dirty = true;
			if (i->second.cbuffer[1])
				i->second.cbuffer[1]->dirty = true;
#if defined(_DEBUG)
			if (m_bindCount >= 1)
			{
				if (i->second.cbuffer[0] && i->second.cbuffer[0]->name == L"cbOnce")
					T_DEBUG(L"cbOnce modified vertex " << i->second.name);
				if (i->second.cbuffer[1] && i->second.cbuffer[1]->name == L"cbOnce")
					T_DEBUG(L"cbOnce modified pixel " << i->second.name);
			}
#endif
		}
	}
}

void ProgramDx11::setVectorParameter(handle_t handle, const Vector4& param)
{
	setVectorArrayParameter(handle, &param, 1);
}

void ProgramDx11::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	SmallMap< handle_t, ParameterMap >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		if (storeIfNotEqual(param, length, &m_parameterFloatArray[i->second.offset]))
		{
			if (i->second.cbuffer[0])
				i->second.cbuffer[0]->dirty = true;
			if (i->second.cbuffer[1])
				i->second.cbuffer[1]->dirty = true;

#if defined(_DEBUG)
			if (m_bindCount >= 1)
			{
				if (i->second.cbuffer[0] && i->second.cbuffer[0]->name == L"cbOnce")
					T_DEBUG(L"cbOnce modified vertex " << i->second.name);
				if (i->second.cbuffer[1] && i->second.cbuffer[1]->name == L"cbOnce")
					T_DEBUG(L"cbOnce modified pixel " << i->second.name);
			}
#endif
		}
	}
}

void ProgramDx11::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	SmallMap< handle_t, ParameterMap >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		param.storeAligned(&m_parameterFloatArray[i->second.offset]);
		if (i->second.cbuffer[0])
			i->second.cbuffer[0]->dirty = true;
		if (i->second.cbuffer[1])
			i->second.cbuffer[1]->dirty = true;
#if defined(_DEBUG)
		if (m_bindCount >= 1)
		{
			if (i->second.cbuffer[0] && i->second.cbuffer[0]->name == L"cbOnce")
				T_DEBUG(L"cbOnce modified vertex " << i->second.name);
			if (i->second.cbuffer[1] && i->second.cbuffer[1]->name == L"cbOnce")
				T_DEBUG(L"cbOnce modified pixel " << i->second.name);
		}
#endif
	}
}

void ProgramDx11::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	SmallMap< handle_t, ParameterMap >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		for (int j = 0; j < length; ++j)
			param[j].storeAligned(&m_parameterFloatArray[i->second.offset + j * 16]);
		if (i->second.cbuffer[0])
			i->second.cbuffer[0]->dirty = true;
		if (i->second.cbuffer[1])
			i->second.cbuffer[1]->dirty = true;
#if defined(_DEBUG)
		if (m_bindCount >= 1)
		{
			if (i->second.cbuffer[0] && i->second.cbuffer[0]->name == L"cbOnce")
				T_DEBUG(L"cbOnce modified vertex " << i->second.name);
			if (i->second.cbuffer[1] && i->second.cbuffer[1]->name == L"cbOnce")
				T_DEBUG(L"cbOnce modified pixel " << i->second.name);
		}
#endif
	}
}

void ProgramDx11::setTextureParameter(handle_t handle, ITexture* texture)
{
	SmallMap< handle_t, ParameterMap >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		m_parameterTextureArray[i->second.offset] = texture;
		m_parameterTextureArrayDirty = true;
	}
}

void ProgramDx11::setStencilReference(uint32_t stencilReference)
{
	m_stencilReference = stencilReference;
}

bool ProgramDx11::bind(
	ID3D11Device* d3dDevice,
	ID3D11DeviceContext* d3dDeviceContext,
	StateCache& stateCache,
	uint32_t d3dInputElementsHash,
	const std::vector< D3D11_INPUT_ELEMENT_DESC >& d3dInputElements,
	const int32_t targetSize[2]
)
{
	// Set states.
	stateCache.setRasterizerState(m_d3dRasterizerState);
	stateCache.setDepthStencilState(m_d3dDepthStencilState, m_stencilReference);
	stateCache.setBlendState(m_d3dBlendState);

	// Update target size parameter.
	setVectorParameter(s_targetSizeHandle, Vector4(float(targetSize[0]), float(targetSize[1]), 0.0f, 0.0f));

	// Update constant buffers.
	if (!updateStateConstants(d3dDeviceContext, m_vertexState))
		return false;
	if (!updateStateConstants(d3dDeviceContext, m_pixelState))
		return false;

	// Bind constant buffers.
	ID3D11Buffer* d3dVSBuffers[] = { m_vertexState.cbuffer[0].d3dBuffer, m_vertexState.cbuffer[1].d3dBuffer, m_vertexState.cbuffer[2].d3dBuffer };
	d3dDeviceContext->VSSetConstantBuffers(0, 3, d3dVSBuffers);

	ID3D11Buffer* d3dPSBuffers[] = { m_pixelState.cbuffer[0].d3dBuffer, m_pixelState.cbuffer[1].d3dBuffer, m_pixelState.cbuffer[2].d3dBuffer };
	d3dDeviceContext->PSSetConstantBuffers(0, 3, d3dPSBuffers);

	// Bind samplers.
	if (!m_vertexState.d3dSamplerStates.empty())
		d3dDeviceContext->VSSetSamplers(
			0,
			UINT(m_vertexState.d3dSamplerStates.size()),
			m_vertexState.d3dSamplerStates.base()
		);

	if (!m_pixelState.d3dSamplerStates.empty())
		d3dDeviceContext->PSSetSamplers(
			0,
			UINT(m_pixelState.d3dSamplerStates.size()),
			m_pixelState.d3dSamplerStates.base()
		);

	// Bind resource views.
	if (m_parameterTextureArrayDirty || ms_activeProgram != this)
	{
		// Unbind previous program's resources.
		if (ms_activeProgram && ms_activeProgram != this)
		{
			ID3D11ShaderResourceView* res = 0;

			for (AlignedVector< std::pair< UINT, uint32_t > >::const_iterator i = ms_activeProgram->m_vertexState.resourceIndices.begin(); i != ms_activeProgram->m_vertexState.resourceIndices.end(); ++i)
				d3dDeviceContext->VSSetShaderResources(i->first, 1, &res);

			for (AlignedVector< std::pair< UINT, uint32_t > >::const_iterator i = ms_activeProgram->m_pixelState.resourceIndices.begin(); i != ms_activeProgram->m_pixelState.resourceIndices.end(); ++i)
				d3dDeviceContext->PSSetShaderResources(i->first, 1, &res);
		}

		// Bind this program's resources.
		for (AlignedVector< std::pair< UINT, uint32_t > >::const_iterator i = m_vertexState.resourceIndices.begin(); i != m_vertexState.resourceIndices.end(); ++i)
		{
			ITexture* texture = m_parameterTextureArray[i->second];
			if (!texture)
				continue;

			Ref< ITexture > resolved = texture->resolve();
			if (!resolved)
				continue;

			ID3D11ShaderResourceView* d3dTextureResourceView;
			if (is_a< SimpleTextureDx11 >(resolved))
				d3dTextureResourceView = static_cast< SimpleTextureDx11* >(resolved.ptr())->getD3D11TextureResourceView();
			else if (is_a< CubeTextureDx11 >(resolved))
				d3dTextureResourceView = static_cast< CubeTextureDx11* >(resolved.ptr())->getD3D11TextureResourceView();
			else if (is_a< RenderTargetDx11 >(resolved))
				d3dTextureResourceView = static_cast< RenderTargetDx11* >(resolved.ptr())->getD3D11TextureResourceView();
			else if (is_a< VolumeTextureDx11 >(resolved))
				d3dTextureResourceView = static_cast< VolumeTextureDx11* >(resolved.ptr())->getD3D11TextureResourceView();
			else
				continue;

			d3dDeviceContext->VSSetShaderResources(i->first, 1, &d3dTextureResourceView);
		}

		for (AlignedVector< std::pair< UINT, uint32_t > >::const_iterator i = m_pixelState.resourceIndices.begin(); i != m_pixelState.resourceIndices.end(); ++i)
		{
			ITexture* texture = m_parameterTextureArray[i->second];
			if (!texture)
				continue;

			Ref< ITexture > resolved = texture->resolve();
			if (!resolved)
				continue;

			ID3D11ShaderResourceView* d3dTextureResourceView;
			if (is_a< SimpleTextureDx11 >(resolved))
				d3dTextureResourceView = static_cast< SimpleTextureDx11* >(resolved.ptr())->getD3D11TextureResourceView();
			else if (is_a< CubeTextureDx11 >(resolved))
				d3dTextureResourceView = static_cast< CubeTextureDx11* >(resolved.ptr())->getD3D11TextureResourceView();
			else if (is_a< RenderTargetDx11 >(resolved))
				d3dTextureResourceView = static_cast< RenderTargetDx11* >(resolved.ptr())->getD3D11TextureResourceView();
			else if (is_a< VolumeTextureDx11 >(resolved))
				d3dTextureResourceView = static_cast< VolumeTextureDx11* >(resolved.ptr())->getD3D11TextureResourceView();
			else
				continue;

			d3dDeviceContext->PSSetShaderResources(i->first, 1, &d3dTextureResourceView);
		}

		m_parameterTextureArrayDirty = false;
	}

	// Bind shaders.
	stateCache.setVertexShader(m_d3dVertexShader);
	stateCache.setPixelShader(m_d3dPixelShader);
	stateCache.setInputLayout(m_d3dVertexShaderHash, m_d3dVertexShaderBlob, d3dInputElementsHash, d3dInputElements);

#if defined(_DEBUG)
	++m_bindCount;
#endif

	ms_activeProgram = this;
	return true;
}

void ProgramDx11::unbind(
	ID3D11Device* d3dDevice,
	ID3D11DeviceContext* d3dDeviceContext
)
{
	ms_activeProgram = 0;
}

bool ProgramDx11::createState(
	ID3D11Device* d3dDevice,
	ResourceCache& resourceCache,
	int32_t shaderType,
	ID3DBlob* d3dShaderBlob,
	const std::map< std::wstring, D3D11_SAMPLER_DESC >& d3dSamplers,
	State& outState
)
{
	ComRef< ID3D11ShaderReflection > d3dShaderReflection;
	ID3D11ShaderReflectionConstantBuffer* d3dConstantBufferReflection;
	ID3D11ShaderReflectionVariable* d3dVariableReflection;
	ID3D11ShaderReflectionType* d3dTypeReflection;
	D3D11_SHADER_DESC dsd;
	D3D11_SHADER_BUFFER_DESC dsbd;
	D3D11_SHADER_VARIABLE_DESC dsvd;
	D3D11_SHADER_TYPE_DESC dstd;
	D3D11_SHADER_INPUT_BIND_DESC dsibd;
	D3D11_BUFFER_DESC dbd;
	HRESULT hr;

	hr = D3DReflect(
		d3dShaderBlob->GetBufferPointer(),
		d3dShaderBlob->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		(void**)&d3dShaderReflection.getAssign()
	);
	if (FAILED(hr))
		return false;

	d3dShaderReflection->GetDesc(&dsd);

	// Scalar parameters.
	for (UINT i = 0; i < dsd.ConstantBuffers; ++i)
	{
		d3dConstantBufferReflection = d3dShaderReflection->GetConstantBufferByIndex(i);
		T_ASSERT (d3dConstantBufferReflection);

		d3dConstantBufferReflection->GetDesc(&dsbd);
		T_ASSERT ((dsbd.Size & 3) == 0);

		dbd.ByteWidth = dsbd.Size;
		dbd.Usage = D3D11_USAGE_DYNAMIC;
		dbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		dbd.MiscFlags = 0;

		hr = d3dDevice->CreateBuffer(&dbd, NULL, &outState.cbuffer[i].d3dBuffer.getAssign());
		if (FAILED(hr))
			return false;

#if defined(_DEBUG)
		outState.cbuffer[i].name = mbstows(dsbd.Name);
#endif

		for (UINT j = 0; j < dsbd.Variables; ++j)
		{
			d3dVariableReflection = d3dConstantBufferReflection->GetVariableByIndex(j);
			T_ASSERT (d3dVariableReflection);

			d3dTypeReflection = d3dVariableReflection->GetType();
			T_ASSERT (d3dTypeReflection);

			d3dVariableReflection->GetDesc(&dsvd);
			T_ASSERT ((dsvd.StartOffset & 3) == 0);

			d3dTypeReflection->GetDesc(&dstd);
			T_ASSERT (dstd.Type == D3D10_SVT_FLOAT);

			SmallMap< handle_t, ParameterMap >::iterator it = m_parameterMap.find(getParameterHandle(mbstows(dsvd.Name)));
			if (it == m_parameterMap.end())
			{
				uint32_t parameterOffset = alignUp(uint32_t(m_parameterFloatArray.size()), 4);
				uint32_t parameterCount = dsvd.Size >> 2;

				m_parameterFloatArray.resize(parameterOffset + parameterCount, 0.0f);

				outState.cbuffer[i].parameterOffsets.push_back(ParameterOffset(
					dsvd.StartOffset,
					parameterOffset,
					parameterCount
				));

				ParameterMap& pm = m_parameterMap[getParameterHandle(mbstows(dsvd.Name))];
#if defined(_DEBUG)
				pm.name = mbstows(dsvd.Name);
#endif
				pm.offset = parameterOffset;
				pm.cbuffer[shaderType] = &outState.cbuffer[i];
			}
			else
			{
				uint32_t parameterOffset = it->second.offset;
				uint32_t parameterCount = dsvd.Size >> 2;

				outState.cbuffer[i].parameterOffsets.push_back(ParameterOffset(
					dsvd.StartOffset,
					parameterOffset,
					parameterCount
				));

				it->second.cbuffer[shaderType] = &outState.cbuffer[i];
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

			SmallMap< handle_t, ParameterMap >::iterator it = m_parameterMap.find(getParameterHandle(mbstows(dsibd.Name)));
			if (it == m_parameterMap.end())
			{
				uint32_t resourceIndex = uint32_t(m_parameterTextureArray.size());
				m_parameterTextureArray.resize(resourceIndex + 1);

				ParameterMap& pm = m_parameterMap[getParameterHandle(mbstows(dsibd.Name))];
#if defined(_DEBUG)
				pm.name = mbstows(dsibd.Name);
#endif
				pm.offset = resourceIndex;

				outState.resourceIndices.push_back(std::make_pair(dsibd.BindPoint, resourceIndex));
			}
			else
			{
				outState.resourceIndices.push_back(std::make_pair(dsibd.BindPoint, it->second.offset));
			}
		}
		else if (dsibd.Type == D3D10_SIT_SAMPLER)
		{
			std::map< std::wstring, D3D11_SAMPLER_DESC >::const_iterator it = d3dSamplers.find(mbstows(dsibd.Name));
			if (it == d3dSamplers.end())
				return false;

			ID3D11SamplerState* d3dSamplerState = resourceCache.getSamplerState(it->second);
			if (!d3dSamplerState)
				return false;

			outState.d3dSamplerStates.push_back(d3dSamplerState);
		}
	}

	return true;
}

bool ProgramDx11::updateStateConstants(ID3D11DeviceContext* d3dDeviceContext, State& state)
{
	D3D11_MAPPED_SUBRESOURCE dm;
	HRESULT hr;

	for (int32_t i = 0; i < sizeof_array(state.cbuffer); ++i)
	{
		if (!state.cbuffer[i].d3dBuffer)
			break;
		if (!state.cbuffer[i].dirty)
			continue;

		hr = d3dDeviceContext->Map(state.cbuffer[i].d3dBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dm);
		if (FAILED(hr))
			return false;

		uint8_t* mapped = (uint8_t*)dm.pData;
		for (AlignedVector< ParameterOffset >::const_iterator j = state.cbuffer[i].parameterOffsets.begin(); j != state.cbuffer[i].parameterOffsets.end(); ++j)
		{
			std::memcpy(
				&mapped[j->constant],
				&m_parameterFloatArray[j->offset],
				j->count * sizeof(float)
			);
		}

		d3dDeviceContext->Unmap(state.cbuffer[i].d3dBuffer, 0);
	}

	return true;
}

	}
}
