/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Dx11/Platform.h"
#include "Render/Resource/ProgramResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Blob;

/*!
 * \ingroup DX11
 */
class T_DLLCLASS ProgramResourceDx11 : public ProgramResource
{
	T_RTTI_CLASS;

public:
	struct ParameterDesc
	{
		std::wstring name;
		uint32_t offset = 0;
		uint32_t count = 0;

		void serialize(ISerializer& s);
	};

	struct ParameterMappingDesc
	{
		uint32_t cbufferOffset = 0;
		uint32_t parameterOffset = 0;
		uint32_t parameterCount = 0;

		ParameterMappingDesc() = default;

		explicit ParameterMappingDesc(uint32_t cbufferOffset_, uint32_t parameterOffset_, uint32_t parameterCount_)
		:	cbufferOffset(cbufferOffset_)
		,	parameterOffset(parameterOffset_)
		,	parameterCount(parameterCount_)
		{
		}

		void serialize(ISerializer& s);
	};

	struct CBufferDesc
	{
		uint32_t size = 0;
		AlignedVector< ParameterMappingDesc > parameters;

		void serialize(ISerializer& s);
	};

	struct ResourceBindingDesc
	{
		uint32_t bindPoint = 0;
		uint32_t parameterOffset = 0;

		ResourceBindingDesc() = default;

		explicit ResourceBindingDesc(uint32_t bindPoint_, uint32_t parameterOffset_)
		:	bindPoint(bindPoint_)
		,	parameterOffset(parameterOffset_)
		{
		}

		void serialize(ISerializer& s);
	};

	ProgramResourceDx11() = default;

	virtual void serialize(ISerializer& s);

private:
	friend class ProgramDx11;
	friend class ProgramCompilerDx11;

	Ref< Blob > m_vertexShader;
	Ref< Blob > m_pixelShader;

	uint32_t m_vertexShaderHash = 0;
	uint32_t m_pixelShaderHash = 0;

	CBufferDesc m_vertexCBuffers[3];
	CBufferDesc m_pixelCBuffers[3];

	AlignedVector< ResourceBindingDesc > m_vertexTextureBindings;
	AlignedVector< ResourceBindingDesc > m_pixelTextureBindings;

	AlignedVector< ResourceBindingDesc > m_vertexStructBufferBindings;
	AlignedVector< ResourceBindingDesc > m_pixelStructBufferBindings;

	AlignedVector< D3D11_SAMPLER_DESC > m_vertexSamplers;
	AlignedVector< D3D11_SAMPLER_DESC > m_pixelSamplers;

	AlignedVector< ParameterDesc > m_parameters;
	uint32_t m_parameterScalarSize = 0;
	uint32_t m_parameterTextureSize = 0;
	uint32_t m_parameterStructBufferSize = 0;

	D3D11_RASTERIZER_DESC m_d3dRasterizerDesc = {};
	D3D11_DEPTH_STENCIL_DESC m_d3dDepthStencilDesc = {};
	D3D11_BLEND_DESC m_d3dBlendDesc = {};
	uint32_t m_stencilReference = 0;
};

	}
}

