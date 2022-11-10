/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace render
	{

class IProgramHints;
class ProgramResource;
class ShaderGraph;

/*! Program compiler interface.
 * \ingroup Render
 */
class T_DLLCLASS IProgramCompiler : public Object
{
	T_RTTI_CLASS;

public:
	/*! Compiled stats. */
	struct Stats
	{
		uint32_t vertexCost;
		uint32_t pixelCost;
		uint32_t vertexSize;
		uint32_t pixelSize;

		Stats()
		:	vertexCost(0)
		,	pixelCost(0)
		,	vertexSize(0)
		,	pixelSize(0)
		{
		}
	};

	/*! Get renderer signature.
	 *
	 * \return Renderer signature.
	 */
	virtual const wchar_t* getRendererSignature() const = 0;

	/*! Compile program.
	 *
	 * \param shaderGraph Program shader graph.
	 * \param settings Compiler settings.
	 * \param name Program name, useful for debugging.
	 * \param outStats Optional stats.
	 * \return Compiled program resource.
	 */
	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		const std::wstring& name,
		Stats* outStats
	) const = 0;

	/*! Generate render specific shader if possible.
	 *
	 * \note This is only used to aid optimization of shader graphs
	 * from within the editor.
	 *
	 * \param shaderGraph Program shader graph.
	 * \param settings Compiler settings.
	 * \param name Program name, useful for debugging.
	 * \param outVertexShader Output render specific shader.
	 * \param outPixelShader Output render specific shader.
	 * \param outComputeShader Output compute specific shader.
	 * \return True if shader was successfully generated.
	 */
	virtual bool generate(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		const std::wstring& name,
		std::wstring& outVertexShader,
		std::wstring& outPixelShader,
		std::wstring& outComputeShader
	) const = 0;
};

	}
}

