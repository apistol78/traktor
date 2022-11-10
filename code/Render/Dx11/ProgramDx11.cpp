/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Matrix44.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/BufferViewDx11.h"
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
,	m_parameterBufferViewArrayDirty(false)
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

			dbd.ByteWidth = alignUp(resource->m_vertexCBuffers[i].size, 16);
			dbd.Usage = D3D11_USAGE_DYNAMIC;
			dbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			dbd.MiscFlags = 0;

			hr = d3dDevice->CreateBuffer(&dbd, NULL, &m_vertexState.cbuffer[i].d3dBuffer.getAssign());
			if (FAILED(hr))
				return false;

			m_vertexState.cbuffer[i].parameterOffsets.reserve(resource->m_vertexCBuffers[i].parameters.size());
			for (const auto& parameter : resource->m_vertexCBuffers[i].parameters)
			{
				m_vertexState.cbuffer[i].parameterOffsets.push_back(ParameterOffset(
					parameter.cbufferOffset,
					parameter.parameterOffset,
					parameter.parameterCount
				));
			}
		}

		for (const auto& vertexTextureBinding : resource->m_vertexTextureBindings)
		{
			m_vertexState.textureResourceIndices.push_back(std::make_pair(
				vertexTextureBinding.bindPoint,
				vertexTextureBinding.parameterOffset
			));
		}

		for (const auto& vertexStructBufferBinding : resource->m_vertexStructBufferBindings)
		{
			m_vertexState.structBufferResourceIndices.push_back(std::make_pair(
				vertexStructBufferBinding.bindPoint,
				vertexStructBufferBinding.parameterOffset
			));
		}

		for (const auto& vertexSampler : resource->m_vertexSamplers)
		{
			ID3D11SamplerState* d3dSamplerState = resourceCache.getSamplerState(vertexSampler);
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

			dbd.ByteWidth = alignUp(resource->m_pixelCBuffers[i].size, 16);
			dbd.Usage = D3D11_USAGE_DYNAMIC;
			dbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			dbd.MiscFlags = 0;

			hr = d3dDevice->CreateBuffer(&dbd, NULL, &m_pixelState.cbuffer[i].d3dBuffer.getAssign());
			if (FAILED(hr))
				return false;

			m_pixelState.cbuffer[i].parameterOffsets.reserve(resource->m_pixelCBuffers[i].parameters.size());
			for (const auto& parameter : resource->m_pixelCBuffers[i].parameters)
			{
				m_pixelState.cbuffer[i].parameterOffsets.push_back(ParameterOffset(
					parameter.cbufferOffset,
					parameter.parameterOffset,
					parameter.parameterCount
				));
			}
		}

		for (const auto& pixelTextureBinding : resource->m_pixelTextureBindings)
		{
			m_pixelState.textureResourceIndices.push_back(std::make_pair(
				pixelTextureBinding.bindPoint,
				pixelTextureBinding.parameterOffset
			));
		}

		for (const auto& pixelStructBufferBinding : resource->m_pixelStructBufferBindings)
		{
			m_pixelState.structBufferResourceIndices.push_back(std::make_pair(
				pixelStructBufferBinding.bindPoint,
				pixelStructBufferBinding.parameterOffset
			));
		}

		for (const auto& pixelSampler : resource->m_pixelSamplers)
		{
			ID3D11SamplerState* d3dSamplerState = resourceCache.getSamplerState(pixelSampler);
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
	m_parameterBufferViewArray.resize(resource->m_parameterStructBufferSize, nullptr);

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
	auto i = m_parameterMap.find(handle);
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
	auto i = m_parameterMap.find(handle);
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
	auto i = m_parameterMap.find(handle);
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
	auto i = m_parameterMap.find(handle);
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
	auto i = m_parameterMap.find(handle);
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
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		m_parameterTextureArray[i->second.offset] = texture;
		m_parameterTextureArrayDirty = true;
	}
}

void ProgramDx11::setImageViewParameter(handle_t handle, ITexture* imageView)
{
}

void ProgramDx11::setBufferViewParameter(handle_t handle, const IBufferView* bufferView)
{
	auto i = m_parameterMap.find(handle);
	if (i != m_parameterMap.end())
	{
		m_parameterBufferViewArray[i->second.offset] = (const BufferViewDx11*)bufferView;
		m_parameterBufferViewArrayDirty = true;
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
	const AlignedVector< D3D11_INPUT_ELEMENT_DESC >& d3dInputElements,
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
	if (m_parameterTextureArrayDirty || m_parameterBufferViewArrayDirty || stateCache.getActiveProgram() != this)
	{
		// Bind this program's resources.
		if (!m_vertexState.textureResourceIndices.empty() || !m_vertexState.structBufferResourceIndices.empty())
		{
			ID3D11ShaderResourceView* d3dVSResourceViews[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

			// Texture resource views.
			for (const auto& resourceIndex : m_vertexState.textureResourceIndices)
			{
				ITexture* texture = m_parameterTextureArray[resourceIndex.second];
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

				d3dVSResourceViews[resourceIndex.first] = d3dTextureResourceView;
			}

			// Struct buffer resource views.
			for (const auto& resourceIndex : m_vertexState.structBufferResourceIndices)
			{
				const BufferViewDx11* bufferView = m_parameterBufferViewArray[resourceIndex.second];
				if (bufferView)
					d3dVSResourceViews[resourceIndex.first] = bufferView->getD3D11ShaderResourceView();
			}

			d3dDeviceContext->VSSetShaderResources(0, sizeof_array(d3dVSResourceViews), d3dVSResourceViews);
		}

		if (!m_pixelState.textureResourceIndices.empty() || !m_pixelState.structBufferResourceIndices.empty())
		{
			ID3D11ShaderResourceView* d3dPSResourceViews[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

			// Texture resource views.
			for (const auto& resourceIndex : m_pixelState.textureResourceIndices)
			{
				ITexture* texture = m_parameterTextureArray[resourceIndex.second];
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

				d3dPSResourceViews[resourceIndex.first] = d3dTextureResourceView;
			}
			
			// Struct buffer resource views.
			for (const auto& resourceIndex : m_pixelState.structBufferResourceIndices)
			{
				const BufferViewDx11* bufferView = m_parameterBufferViewArray[resourceIndex.second];
				if (bufferView)
					d3dPSResourceViews[resourceIndex.first] = bufferView->getD3D11ShaderResourceView();
			}

			d3dDeviceContext->PSSetShaderResources(0, sizeof_array(d3dPSResourceViews), d3dPSResourceViews);
		}

		m_parameterTextureArrayDirty = false;
		m_parameterBufferViewArrayDirty = false;
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
			for (const auto& parameterOffset : state.cbuffer[i].parameterOffsets)
			{
				std::memcpy(
					&mapped[parameterOffset.constant],
					&m_parameterFloatArray[parameterOffset.offset],
					parameterOffset.count * sizeof(float)
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
