#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/CubeTextureDx11.h"
#include "Render/Dx11/ProgramDx11.h"
#include "Render/Dx11/ProgramResourceDx11.h"
#include "Render/Dx11/RenderTargetDepthDx11.h"
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
		if (dest[i] != source[i])
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
		if (Vector4::loadAligned(&dest[i * 4]) != source[i])
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
	D3D11_BUFFER_DESC dbd;
	HRESULT hr;

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

	// Create vertex states.
	{
		for (uint32_t i = 0; i < sizeof_array(resource->m_vertexCBuffers); ++i)
		{
			if (!resource->m_vertexCBuffers[i].size)
				continue;

			dbd.ByteWidth = resource->m_vertexCBuffers[i].size;
			dbd.Usage = D3D11_USAGE_DYNAMIC;
			dbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			dbd.MiscFlags = 0;
		
			hr = d3dDevice->CreateBuffer(&dbd, NULL, &m_vertexState.cbuffer[i].d3dBuffer.getAssign());
			if (FAILED(hr))
				return false;

			m_vertexState.cbuffer[i].parameterOffsets.reserve(resource->m_vertexCBuffers[i].parameters.size());
			for (uint32_t j = 0; j < uint32_t(resource->m_vertexCBuffers[i].parameters.size()); ++j)
			{
				m_vertexState.cbuffer[i].parameterOffsets.push_back(ParameterOffset(
					resource->m_vertexCBuffers[i].parameters[j].cbufferOffset,
					resource->m_vertexCBuffers[i].parameters[j].parameterOffset,
					resource->m_vertexCBuffers[i].parameters[j].parameterCount
				));
			}
		}

		for (uint32_t i = 0; i < uint32_t(resource->m_vertexTextureBindings.size()); ++i)
		{
			m_vertexState.resourceIndices.push_back(std::make_pair(
				resource->m_vertexTextureBindings[i].bindPoint,
				resource->m_vertexTextureBindings[i].parameterOffset
			));
		}

		for (uint32_t i = 0; i < uint32_t(resource->m_vertexSamplers.size()); ++i)
		{
			ID3D11SamplerState* d3dSamplerState = resourceCache.getSamplerState(resource->m_vertexSamplers[i]);
			if (!d3dSamplerState)
				return false;

			m_vertexState.d3dSamplerStates.push_back(d3dSamplerState);
		}
	}

	// Create pixel states.
	{
		for (uint32_t i = 0; i < sizeof_array(resource->m_pixelCBuffers); ++i)
		{
			if (!resource->m_pixelCBuffers[i].size)
				continue;

			dbd.ByteWidth = resource->m_pixelCBuffers[i].size;
			dbd.Usage = D3D11_USAGE_DYNAMIC;
			dbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			dbd.MiscFlags = 0;
		
			hr = d3dDevice->CreateBuffer(&dbd, NULL, &m_pixelState.cbuffer[i].d3dBuffer.getAssign());
			if (FAILED(hr))
				return false;

			m_pixelState.cbuffer[i].parameterOffsets.reserve(resource->m_pixelCBuffers[i].parameters.size());
			for (uint32_t j = 0; j < uint32_t(resource->m_pixelCBuffers[i].parameters.size()); ++j)
			{
				m_pixelState.cbuffer[i].parameterOffsets.push_back(ParameterOffset(
					resource->m_pixelCBuffers[i].parameters[j].cbufferOffset,
					resource->m_pixelCBuffers[i].parameters[j].parameterOffset,
					resource->m_pixelCBuffers[i].parameters[j].parameterCount
				));
			}
		}

		for (uint32_t i = 0; i < uint32_t(resource->m_pixelTextureBindings.size()); ++i)
		{
			m_pixelState.resourceIndices.push_back(std::make_pair(
				resource->m_pixelTextureBindings[i].bindPoint,
				resource->m_pixelTextureBindings[i].parameterOffset
			));
		}

		for (uint32_t i = 0; i < uint32_t(resource->m_pixelSamplers.size()); ++i)
		{
			ID3D11SamplerState* d3dSamplerState = resourceCache.getSamplerState(resource->m_pixelSamplers[i]);
			if (!d3dSamplerState)
				return false;

			m_pixelState.d3dSamplerStates.push_back(d3dSamplerState);
		}
	}

	// Setup shared parameters.
	for (uint32_t i = 0; i < uint32_t(resource->m_parameters.size()); ++i)
	{
		ParameterMap& pm = m_parameterMap[getParameterHandle(resource->m_parameters[i].name)];
#if defined(_DEBUG)
		pm.name = resource->m_parameters[i].name;
#endif
		pm.offset = resource->m_parameters[i].offset;
		pm.count = resource->m_parameters[i].count;
	}

	m_parameterFloatArray.resize(resource->m_parameterScalarSize, 0.0f);
	m_parameterTextureArray.resize(resource->m_parameterTextureSize);

	// Create state objects.
	m_d3dRasterizerState = resourceCache.getRasterizerState(resource->m_d3dRasterizerDesc);
	m_d3dDepthStencilState = resourceCache.getDepthStencilState(resource->m_d3dDepthStencilDesc);
	m_d3dBlendState = resourceCache.getBlendState(resource->m_d3dBlendDesc);

	m_stencilReference = resource->m_stencilReference;
	return true;
}

void ProgramDx11::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());

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

	m_d3dVertexShaderBlob = 0;

	m_parameterMap.clear();
	m_parameterFloatArray.resize(0);
	m_parameterTextureArray.resize(0);
}

void ProgramDx11::setFloatParameter(handle_t handle, float param)
{
	SmallMap< handle_t, ParameterMap >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		if (storeIfNotEqual(&param, 1, &m_parameterFloatArray[i->second.offset]))
		{
			if (i->second.cbuffer[0])
				i->second.cbuffer[0]->dirty = true;
			if (i->second.cbuffer[1])
				i->second.cbuffer[1]->dirty = true;
		}
	}
}

void ProgramDx11::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	SmallMap< handle_t, ParameterMap >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		float* out = &m_parameterFloatArray[i->second.offset];
		for (int j = 0; j < length - 1; ++j)
		{
			Vector4(Scalar(param[j])).storeAligned(out);
			out += 4;
		}
		*out++ = param[length - 1];

		if (i->second.cbuffer[0])
			i->second.cbuffer[0]->dirty = true;
		if (i->second.cbuffer[1])
			i->second.cbuffer[1]->dirty = true;
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
		T_FATAL_ASSERT (length * 4 <= i->second.count);
		if (storeIfNotEqual(param, length, &m_parameterFloatArray[i->second.offset]))
		{
			if (i->second.cbuffer[0])
				i->second.cbuffer[0]->dirty = true;
			if (i->second.cbuffer[1])
				i->second.cbuffer[1]->dirty = true;
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
	}
}

void ProgramDx11::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	SmallMap< handle_t, ParameterMap >::iterator i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		T_FATAL_ASSERT (length * 16 <= i->second.count);
		for (int j = 0; j < length; ++j)
			param[j].storeAligned(&m_parameterFloatArray[i->second.offset + j * 16]);
		if (i->second.cbuffer[0])
			i->second.cbuffer[0]->dirty = true;
		if (i->second.cbuffer[1])
			i->second.cbuffer[1]->dirty = true;
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
	if (m_parameterTextureArrayDirty || stateCache.getActiveProgram() != this)
	{
		//// Unbind previous program's resources.
		//if (stateCache.getActiveProgram() && stateCache.getActiveProgram() != this)
		//{
		//	ID3D11ShaderResourceView* res = 0;

		//	for (AlignedVector< std::pair< UINT, uint32_t > >::const_iterator i = ms_activeProgram->m_vertexState.resourceIndices.begin(); i != ms_activeProgram->m_vertexState.resourceIndices.end(); ++i)
		//		d3dDeviceContext->VSSetShaderResources(i->first, 1, &res);

		//	for (AlignedVector< std::pair< UINT, uint32_t > >::const_iterator i = ms_activeProgram->m_pixelState.resourceIndices.begin(); i != ms_activeProgram->m_pixelState.resourceIndices.end(); ++i)
		//		d3dDeviceContext->PSSetShaderResources(i->first, 1, &res);
		//}

		// Bind this program's resources.
		if (!m_vertexState.resourceIndices.empty())
		{
			ID3D11ShaderResourceView* d3dVSTextureResourceViews[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
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
				else if (is_a< RenderTargetDepthDx11 >(resolved))
					d3dTextureResourceView = static_cast< RenderTargetDepthDx11* >(resolved.ptr())->getD3D11TextureResourceView();
				else if (is_a< VolumeTextureDx11 >(resolved))
					d3dTextureResourceView = static_cast< VolumeTextureDx11* >(resolved.ptr())->getD3D11TextureResourceView();
				else
					continue;

				d3dVSTextureResourceViews[i->first] = d3dTextureResourceView;
			}
			d3dDeviceContext->VSSetShaderResources(0, sizeof_array(d3dVSTextureResourceViews), d3dVSTextureResourceViews);
		}

		if (!m_pixelState.resourceIndices.empty())
		{
			ID3D11ShaderResourceView* d3dPSTextureResourceViews[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
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
				else if (is_a< RenderTargetDepthDx11 >(resolved))
					d3dTextureResourceView = static_cast< RenderTargetDepthDx11* >(resolved.ptr())->getD3D11TextureResourceView();
				else if (is_a< VolumeTextureDx11 >(resolved))
					d3dTextureResourceView = static_cast< VolumeTextureDx11* >(resolved.ptr())->getD3D11TextureResourceView();
				else
					continue;

				d3dPSTextureResourceViews[i->first] = d3dTextureResourceView;
			}
			d3dDeviceContext->PSSetShaderResources(0, sizeof_array(d3dPSTextureResourceViews), d3dPSTextureResourceViews);
		}

		m_parameterTextureArrayDirty = false;
	}

	stateCache.setVertexShader(m_d3dVertexShader);
	stateCache.setPixelShader(m_d3dPixelShader);
	stateCache.setInputLayout(m_d3dVertexShaderHash, m_d3dVertexShaderBlob, d3dInputElementsHash, d3dInputElements);
	stateCache.setActiveProgram(this);

#if defined(_DEBUG)
	++m_bindCount;
#endif

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

		if (!state.cbuffer[i].dirty && state.cbuffer[i].d3dMappedContext == d3dDeviceContext)
			continue;

		hr = d3dDeviceContext->Map(state.cbuffer[i].d3dBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dm);
		if (FAILED(hr))
			return false;

		uint8_t* mapped = (uint8_t*)dm.pData;
		if (mapped)
		{
			for (AlignedVector< ParameterOffset >::const_iterator j = state.cbuffer[i].parameterOffsets.begin(); j != state.cbuffer[i].parameterOffsets.end(); ++j)
			{
				std::memcpy(
					&mapped[j->constant],
					&m_parameterFloatArray[j->offset],
					j->count * sizeof(float)
				);
			}
		}

		d3dDeviceContext->Unmap(state.cbuffer[i].d3dBuffer, 0);
		state.cbuffer[i].d3dMappedContext = d3dDeviceContext;
	}

	return true;
}

	}
}
