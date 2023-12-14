/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"
#include "Render/Resource/ProgramResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_VULKAN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*!
 * \ingroup Vulkan
 */
class T_DLLCLASS ProgramResourceVk : public ProgramResource
{
	T_RTTI_CLASS;

public:
	enum BindStages
	{
		BsVertex = 1,
		BsFragment = 2,
		BsCompute = 4
	};

	struct ParameterDesc
	{
		std::wstring name;
		int32_t ubuffer = -1;			//!< Index into which uniform buffer, -1 if not a uniform parameter.
		uint32_t ubufferOffset = 0;		//!< Offset in 4-byte floats.
		uint32_t ubufferSize = 0;		//!< Size in 4-byte floats.
		int32_t textureIndex = -1;
		int32_t imageIndex = -1;
		int32_t sbufferIndex = -1;

		void serialize(ISerializer& s);
	};

	struct SamplerDesc
	{
		int32_t binding = -1;
		uint8_t stages = 0;
		SamplerState state;

		void serialize(ISerializer& s);
	};

	struct TextureDesc
	{
		std::wstring name;
		int32_t binding = -1;
		uint8_t stages = 0;

		void serialize(ISerializer& s);
	};

	 struct ImageDesc
	 {
	 	std::wstring name;
	 	int32_t binding = -1;
	 	uint8_t stages = 0;

	 	void serialize(ISerializer& s);
	 };

	struct SBufferDesc
	{
		std::wstring name;
		int32_t binding = -1;
		uint8_t stages = 0;

		void serialize(ISerializer& s);
	};

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ProgramVk;
	friend class ProgramCompilerVk;

	RenderState m_renderState;

	AlignedVector< uint32_t > m_vertexShader;
	AlignedVector< uint32_t > m_fragmentShader;
	AlignedVector< uint32_t > m_computeShader;

	uint32_t m_uniformBufferSizes[3];	// Once(0), Frame(1) and Draw(2)

	AlignedVector< ParameterDesc > m_parameters;

	AlignedVector< SamplerDesc > m_samplers;
	AlignedVector< TextureDesc > m_textures;
	AlignedVector< ImageDesc > m_images;
	AlignedVector< SBufferDesc > m_sbuffers;

	int32_t m_localWorkGroupSize[3] = { 1, 1, 1 };

	uint32_t m_vertexShaderHash = 0;
	uint32_t m_fragmentShaderHash = 0;
	uint32_t m_computeShaderHash = 0;
	uint32_t m_shaderHash = 0;
	uint32_t m_layoutHash = 0;

	bool m_useTargetSize = false;
};

}
