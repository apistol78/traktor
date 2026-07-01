/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/IdAllocator.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Io/StringOutputStream.h"
#include "Render/Editor/Glsl/GlslType.h"
#include "Render/Editor/Glsl/GlslVariable.h"
#include "Render/Editor/Shader/StructDeclaration.h"
#include "Render/Types.h"

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

}

namespace traktor::render
{

class GlslLayout;
class OutputPin;
class ShaderModule;

/*!
 * \ingroup Render
 */
class T_DLLCLASS GlslShader
{
public:
	enum ShaderType
	{
		StVertex,
		StFragment,
		StCompute,
		StCallable
	};

	enum BlockType
	{
		BtInput,
		BtOutput,
		BtBody,
		BtLast
	};

	explicit GlslShader(ShaderType shaderType, const ShaderModule* shaderModule);

	virtual ~GlslShader();

	void addInputVariable(const std::wstring& variableName, GlslVariable* variable);

	GlslVariable* getInputVariable(const std::wstring& variableName);

	GlslVariable* createTemporaryVariable(const OutputPin* outputPin, GlslType type);

	GlslVariable* createVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type);

	GlslVariable* createArrayVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type);

	GlslVariable* createStructVariable(const OutputPin* outputPin, const std::wstring& variableName, const std::wstring& structTypeName, const StructDeclaration& structDeclaration);

	GlslVariable* createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, GlslType type);

	GlslVariable* getVariable(const OutputPin* outputPin);

	void pushScope();

	void popScope();

	/*! \name Output streams */
	/*! \{ */

	StringOutputStream& pushOutputStream(BlockType blockType, const wchar_t* const tag);

	void popOutputStream(BlockType blockType);

	StringOutputStream& getOutputStream(BlockType blockType);

	const StringOutputStream& getOutputStream(BlockType blockType) const;

	/*! \} */

	void addStructure(const std::wstring& typeName, const StructDeclaration& decl);

	std::wstring getGeneratedShader(
		const PropertyGroup* settings,
		const GlslLayout& layout,
		const GlslRequirements& requirements) const;

private:
	struct OutputPinVariable
	{
		const OutputPin* outputPin;
		Ref< GlslVariable > variable;
		int32_t index;
	};

	struct OutputStreamTuple
	{
		Ref< StringOutputStream > outputStream;
		const wchar_t* tag;
	};

	ShaderType m_shaderType;
	Ref< const ShaderModule > m_shaderModule;
	SmallMap< std::wstring, Ref< GlslVariable > > m_inputVariables;
	AlignedVector< OutputPinVariable > m_variables;
	AlignedVector< uint32_t > m_variableScopes;
	AlignedVector< OutputPinVariable > m_outerVariables;
	IdAllocator m_temporaryVariableAlloc;
	AlignedVector< OutputStreamTuple > m_outputStreams[BtLast];
	SmallMap< std::wstring, StructDeclaration > m_structs;
};

}
