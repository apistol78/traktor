/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Render/Editor/Glsl/GlslEmitter.h"
#include "Render/Editor/Glsl/GlslShader.h"
#include "Render/Editor/Glsl/GlslLayout.h"

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

class ShaderGraph;
class InputPin;
class OutputPin;
class GlslUniforms;
class GlslVariable;

/*!
 * \ingroup Render
 */
class T_DLLCLASS GlslContext
{
public:
	struct Parameter
	{
		std::wstring name;
		ParameterType type;
		int32_t length;
		UpdateFrequency frequency;
	};

	explicit GlslContext(const ShaderGraph* shaderGraph, const PropertyGroup* settings);

	Node* getInputNode(const InputPin* inputPin);

	Node* getInputNode(Node* node, const std::wstring& inputPinName);

	bool emit(Node* node);

	GlslVariable* emitInput(const InputPin* inputPin);

	GlslVariable* emitInput(Node* node, const std::wstring& inputPinName);

	GlslVariable* emitOutput(const OutputPin* outputPin, GlslType type);

	GlslVariable* emitOutput(Node* node, const std::wstring& outputPinName, GlslType type);

	void findNonDependentOutputs(Node* node, const std::wstring& inputPinName, const AlignedVector< const OutputPin* >& dependentOutputPins, AlignedVector< const OutputPin* >& outOutputPins) const;

	void findCommonOutputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, AlignedVector< const OutputPin* >& outOutputPins) const;

	void findCommonOutputs(const AlignedVector< const InputPin* >& inputPins, AlignedVector< const OutputPin* >& outOutputPins) const;

	bool isConnected(const OutputPin* outputPin) const;

	/*! \name Active stage */
	/*! \{ */

	void enterVertex();

	bool inVertex() const;

	void enterFragment();

	bool inFragment() const;

	void enterCompute();

	bool inCompute() const;

	void enterCallable();

	bool inCallable() const;

	GlslShader& getShader() { return *m_currentShader; }

	/*! \} */

	const PropertyGroup* getSettings() const;

	/*! Requirements */
	GlslRequirements& requirements();

	/*! Requirements */
	const GlslRequirements& requirements() const;

	/*! Allocate interpolator variable. */
	bool allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset);

	/*! \name Parameters */
	/*! \{ */

	/*! Define program parameter.
	 *
	 * \param name Name of parameter.
	 * \param type Type of parameter.
	 * \param length Length of array parameter.
	 * \param frequency Expected value change frequency of parameter.
	 * \return True if parameter defined successfully, ie not previously defined.
	 */
	bool addParameter(const std::wstring& name, ParameterType type, int32_t length, UpdateFrequency frequency);

	bool haveParameter(const std::wstring& name) const;

	const AlignedVector< Parameter >& getParameters() const { return m_parameters; }

	/*! \} */


	/*! \name Layout of resources */
	/*! \{ */

	GlslLayout& getLayout();

	/*! \} */


	/*! \name Render state */
	/*! \{ */

	void setRenderState(const RenderState& renderState);

	const RenderState& getRenderState() const { return m_renderState;  }

	/*! \} */


	/*! \name Generated shaders */
	/*! \{ */

	GlslShader& getVertexShader() { return m_vertexShader; }

	GlslShader& getFragmentShader() { return m_fragmentShader; }

	GlslShader& getComputeShader() { return m_computeShader; }

	GlslShader& getCallableShader() { return m_callableShader; }

	GlslEmitter& getEmitter() { return m_emitter; }

	/*! \} */

	/*! \name Error reporting */
	/*! \{ */

	void pushError(const std::wstring& errorMessage);

	std::wstring getErrorReport() const;

	/*! \} */

private:
	struct Scope
	{
		const InputPin* inputPin = nullptr;
		const OutputPin* outputPin = nullptr;

		Scope() = default;

		explicit Scope(const InputPin* inputPin_, const OutputPin* outputPin_)
		:	inputPin(inputPin_)
		,	outputPin(outputPin_)
		{
		}
	};

	struct Error
	{
		std::wstring message;
		std::wstring scope;
	};

	Ref< const ShaderGraph > m_shaderGraph;
	Ref< const PropertyGroup > m_settings;
	GlslShader m_vertexShader;
	GlslShader m_fragmentShader;
	GlslShader m_computeShader;
	GlslShader m_callableShader;
	GlslShader* m_currentShader;
	GlslEmitter m_emitter;
	GlslRequirements m_requirements;
	AlignedVector< uint8_t > m_interpolatorMap;
	GlslLayout m_layout;
	AlignedVector< Parameter > m_parameters;
	RenderState m_renderState;
	std::list< Scope > m_emitScope;
	std::list< Error > m_errorMessages;
	std::wstring m_error;

	std::wstring getCurrentScope() const;
};

}
