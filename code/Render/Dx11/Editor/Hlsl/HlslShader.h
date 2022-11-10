/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include <map>
#include <set>
#include <string>
#include "Core/RefArray.h"
#include "Core/Containers/IdAllocator.h"
#include "Core/Io/StringOutputStream.h"
#include "Render/Types.h"
#include "Render/Dx11/Editor/Hlsl/HlslType.h"
#include "Render/Dx11/Editor/Hlsl/HlslVariable.h"

namespace traktor
{
	namespace render
	{

class OutputPin;

/*!
 * \ingroup DX11
 */
class HlslShader
{
public:
	enum ShaderType
	{
		StVertex,
		StPixel,
		StCompute
	};

	enum BlockType
	{
		BtCBufferOnce,
		BtCBufferFrame,
		BtCBufferDraw,
		BtStructs,
		BtTextures,
		BtSamplers,
		BtInput,
		BtOutput,
		BtScript,
		BtBody,
		BtLast
	};

	HlslShader(ShaderType shaderType);

	virtual ~HlslShader();

	bool haveInput(const std::wstring& inputName) const;

	void addInput(const std::wstring& inputName);

	HlslVariable* createTemporaryVariable(const OutputPin* outputPin, HlslType type);

	HlslVariable* createVariable(const OutputPin* outputPin, const std::wstring& variableName, HlslType type);

	HlslVariable* createOuterVariable(const OutputPin* outputPin, const std::wstring& variableName, HlslType type);

	HlslVariable* getVariable(const OutputPin* outputPin) const;

	void pushScope();

	void popScope();

	int32_t allocateInterpolator();

	int32_t allocateBooleanRegister();

	void allocateVPos();

	void allocateVFace();

	void allocateInstanceID();

	void allocateTargetSize();

	bool defineScript(const std::wstring& signature);

	void addSampler(const std::wstring& sampler, const D3D11_SAMPLER_DESC& dsd);

	const std::map< std::wstring, D3D11_SAMPLER_DESC >& getSamplers() const;

	void addUniform(const std::wstring& uniform);

	const std::set< std::wstring >& getUniforms() const;

	/*! \name Output streams */
	/*! \{ */

	StringOutputStream& pushOutputStream(BlockType blockType, const wchar_t* const tag);

	void popOutputStream(BlockType blockType);

	StringOutputStream& getOutputStream(BlockType blockType);

	const StringOutputStream& getOutputStream(BlockType blockType) const;

	/*! \} */

	std::wstring getGeneratedShader();

private:
	struct OutputPinVariable
	{
		const OutputPin* outputPin;
		Ref< HlslVariable > variable;
		int32_t index;
	};

	struct OutputStreamTuple
	{
		Ref< StringOutputStream > outputStream;
		const wchar_t* tag;
	};

	ShaderType m_shaderType;
	std::set< std::wstring > m_inputs;
	AlignedVector< OutputPinVariable > m_variables;
	AlignedVector< uint32_t > m_variableScopes;
	AlignedVector< OutputPinVariable > m_outerVariables;
	int32_t m_interpolatorCount;
	int32_t m_booleanRegisterCount;
	std::set< std::wstring > m_scripts;
	std::map< std::wstring, D3D11_SAMPLER_DESC > m_samplers;
	std::set< std::wstring > m_uniforms;
	IdAllocator m_temporaryVariableAlloc;
	AlignedVector< OutputStreamTuple > m_outputStreams[BtLast];
	bool m_needVPos;
	bool m_needVFace;
	bool m_needTargetSize;
	bool m_needInstanceID;
};

	}
}

