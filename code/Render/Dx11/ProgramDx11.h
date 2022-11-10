/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Misc/ComRef.h"
#include "Render/IProgram.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

class Blob;
class BufferViewDx11;
class ContextDx11;
class ProgramResourceDx11;
class ResourceCache;
class StateCache;
class HlslProgram;

/*!
 * \ingroup DX11
 */
class ProgramDx11 : public IProgram
{
	T_RTTI_CLASS;

public:
	explicit ProgramDx11(ContextDx11* context);

	virtual ~ProgramDx11();

	bool create(ID3D11Device* d3dDevice, ResourceCache& resourceCache, const ProgramResourceDx11* resource);

	virtual void destroy() override final;

	virtual void setFloatParameter(handle_t handle, float param) override final;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) override final;

	virtual void setVectorParameter(handle_t handle, const Vector4& param) override final;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) override final;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) override final;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) override final;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) override final;

	virtual void setImageViewParameter(handle_t handle, ITexture* imageView) override final;

	virtual void setBufferViewParameter(handle_t handle, const IBufferView* bufferView) override final;

	virtual void setStencilReference(uint32_t stencilReference) override final;

	bool bind(
		ID3D11Device* d3dDevice,
		ID3D11DeviceContext* d3dDeviceContext,
		StateCache& stateCache,
		uint32_t d3dInputElementsHash,
		const AlignedVector< D3D11_INPUT_ELEMENT_DESC >& d3dInputElements,
		const int32_t targetSize[2]
	);

private:
	struct ParameterOffset
	{
		UINT constant = 0;
		uint32_t offset = 0;
		uint32_t count = 0;

		ParameterOffset() = default;

		explicit ParameterOffset(UINT constant_, uint32_t offset_, uint32_t count_)
		:	constant(constant_)
		,	offset(offset_)
		,	count(count_)
		{
		}
	};

	struct CBuffer
	{
#if defined(_DEBUG)
		std::wstring name;
#endif
		ComRef< ID3D11Buffer > d3dBuffer;
		AlignedVector< ParameterOffset > parameterOffsets;
		ID3D11DeviceContext* d3dMappedContext = nullptr;
		bool dirty = true;
	};

	struct ParameterMap
	{
#if defined(_DEBUG)
		std::wstring name;
#endif
		uint32_t offset = 0;
		uint32_t count = 0;
		CBuffer* cbuffer[2] = { nullptr, nullptr };
	};

	struct State
	{
		CBuffer cbuffer[3];
		ComRefArray< ID3D11SamplerState > d3dSamplerStates;
		AlignedVector< std::pair< UINT, uint32_t > > textureResourceIndices;
		AlignedVector< std::pair< UINT, uint32_t > > structBufferResourceIndices;
	};

	Ref< ContextDx11 > m_context;
	ComRef< ID3D11RasterizerState > m_d3dRasterizerState;
	ComRef< ID3D11DepthStencilState > m_d3dDepthStencilState;
	ComRef< ID3D11BlendState > m_d3dBlendState;
	uint32_t m_stencilReference;
	ComRef< ID3D11VertexShader > m_d3dVertexShader;
	ComRef< ID3D11PixelShader > m_d3dPixelShader;
	State m_vertexState;
	State m_pixelState;
	Ref< Blob > m_d3dVertexShaderBlob;
	uint32_t m_d3dVertexShaderHash;
	SmallMap< handle_t, ParameterMap > m_parameterMap;
	AlignedVector< float > m_parameterFloatArray;
	RefArray< ITexture > m_parameterTextureArray;
	AlignedVector< const BufferViewDx11* > m_parameterBufferViewArray;
	bool m_parameterTextureArrayDirty;
	bool m_parameterBufferViewArrayDirty;

#if defined(_DEBUG)
	int32_t m_bindCount;
#endif

	bool updateStateConstants(ID3D11DeviceContext* d3dDeviceContext, State& state);
};

	}
}

