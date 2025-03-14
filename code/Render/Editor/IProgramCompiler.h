/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Render/Types.h"

#include <list>
#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Guid;
class PropertyGroup;

}

namespace traktor::render
{

class IProgramHints;
class ProgramResource;
class ShaderModule;
class ShaderGraph;

/*! Program compiler interface.
 * \ingroup Render
 */
class T_DLLCLASS IProgramCompiler : public Object
{
	T_RTTI_CLASS;

public:
	struct Error
	{
		std::wstring message;
		std::wstring source;
	};

	struct Output
	{
		std::wstring vertex;
		std::wstring pixel;
		std::wstring compute;
	};

	/*!
	 */
	virtual bool create(IProgramCompiler* embedded) = 0;

	/*! Get renderer signature.
	 *
	 * \return Renderer signature.
	 */
	virtual const wchar_t* getRendererSignature() const = 0;

	/*! Compile program.
	 *
	 * \param shaderGraph Program shader graph.
	 * \param shaderModule Program shader module.
	 * \param settings Compiler settings.
	 * \param name Program name, useful for debugging.
	 * \return Compiled program resource.
	 */
	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		const ShaderModule* shaderModule,
		const PropertyGroup* settings,
		const std::wstring& name,
		std::list< Error >& outErrors) const = 0;

	/*! Generate render specific shader if possible.
	 *
	 * \note This is only used to aid optimization of shader graphs
	 * from within the editor.
	 *
	 * \param shaderGraph Program shader graph.
	 * \param shaderModule Program shader module.
	 * \param settings Compiler settings.
	 * \param name Program name, useful for debugging.
	 * \param output Shader outputs.
	 * \return True if shader was successfully generated.
	 */
	virtual bool generate(
		const ShaderGraph* shaderGraph,
		const ShaderModule* shaderModule,
		const PropertyGroup* settings,
		const std::wstring& name,
		Output& output) const = 0;
};

}
