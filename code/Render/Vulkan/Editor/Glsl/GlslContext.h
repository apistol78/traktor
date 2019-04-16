#pragma once

#include "Core/Containers/SmallMap.h"
#include "Render/Vulkan/Editor/Glsl/GlslEmitter.h"
#include "Render/Vulkan/Editor/Glsl/GlslShader.h"
#include "Render/Vulkan/Editor/Glsl/GlslLayout.h"

namespace traktor
{
	namespace render
	{

class ShaderGraph;
class InputPin;
class OutputPin;
class GlslUniforms;
class GlslVariable;

/*!
 * \ingroup Vulkan
 */
class GlslContext
{
public:
	struct Parameter
	{
		std::wstring name;
		ParameterType type;
		int32_t length;
		UpdateFrequency frequency;
	};

	GlslContext(const ShaderGraph* shaderGraph);

	Node* getInputNode(const InputPin* inputPin);

	Node* getInputNode(Node* node, const std::wstring& inputPinName);

	bool emit(Node* node);

	GlslVariable* emitInput(const InputPin* inputPin);

	GlslVariable* emitInput(Node* node, const std::wstring& inputPinName);

	GlslVariable* emitOutput(Node* node, const std::wstring& outputPinName, GlslType type);

	void findNonDependentOutputs(Node* node, const std::wstring& inputPinName, const AlignedVector< const OutputPin* >& dependentOutputPins, AlignedVector< const OutputPin* >& outOutputPins) const;

	void findCommonOutputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, AlignedVector< const OutputPin* >& outOutputPins) const;


	/*! \name Active stage */
	/*! \{ */

	void enterVertex();

	void enterFragment();

	void enterCompute();

	bool inVertex() const;

	bool inFragment() const;

	bool inCompute() const;

	GlslShader& getShader() { return *m_currentShader; }

	/*! \} */


	/*! \brief Allocate interpolator variable. */
	bool allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset);

	/*! \name Parameters */
	/*! \{ */

	/*! \brief Define program parameter.
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

	GlslEmitter& getEmitter() { return m_emitter; }

	/*! \} */

private:
	struct Scope
	{
		const InputPin* inputPin;
		const OutputPin* outputPin;

		Scope()
		:	inputPin(nullptr)
		,	outputPin(nullptr)
		{
		}

		Scope(const InputPin* inputPin_, const OutputPin* outputPin_)
		:	inputPin(inputPin_)
		,	outputPin(outputPin_)
		{
		}
	};

	Ref< const ShaderGraph > m_shaderGraph;
	GlslShader m_vertexShader;
	GlslShader m_fragmentShader;
	GlslShader m_computeShader;
	GlslShader* m_currentShader;
	GlslEmitter m_emitter;
	AlignedVector< uint8_t > m_interpolatorMap;
	GlslLayout m_layout;
	AlignedVector< Parameter > m_parameters;
	RenderState m_renderState;
	std::list< Scope > m_emitScope;
	std::wstring m_error;
};

	}
}

