#include "Core/Log/Log.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/ShaderGraphTraverse.h"
#include "Render/Vulkan/Glsl/GlslContext.h"
#include "Render/Vulkan/Glsl/GlslShader.h"

namespace traktor
{
	namespace render
	{

GlslContext::GlslContext(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
,	m_vertexShader(GlslShader::StVertex)
,	m_fragmentShader(GlslShader::StFragment)
,	m_currentShader(0)
{
}

Node* GlslContext::getInputNode(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	return sourcePin ? sourcePin->getNode() : 0;
}

Node* GlslContext::getInputNode(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT (inputPin);

	return getInputNode(inputPin);
}

void GlslContext::emit(Node* node)
{
	int32_t outputPinCount = node->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		GlslVariable* variable = m_currentShader->getVariable(node->getOutputPin(i));
		if (!variable)
		{
			if (!m_emitter.emit(*this, node))
			{
				log::error << L"Failed to emit " << type_name(node) << Endl;
				log::error << L"  " << type_name(node) << L"[" << node->getOutputPin(i)->getName() << L"] " << node->getInformation() << Endl;
			}
			break;
		}
	}
}

GlslVariable* GlslContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	GlslVariable* variable = m_currentShader->getVariable(sourcePin);
	if (!variable)
	{
		Node* node = sourcePin->getNode();
		if (m_emitter.emit(*this, node))
		{
			variable = m_currentShader->getVariable(sourcePin);
			T_ASSERT (variable);
		}
		else
		{
			log::error << L"Failed to emit " << type_name(node) << Endl;
			log::error << L"  " << type_name(node) << L"[" << sourcePin->getName() << L"] " << node->getInformation() << L" -->" << Endl;
			log::error << L"  " << type_name(inputPin->getNode()) << L"[" << inputPin->getName() << L"] " << inputPin->getNode()->getInformation() << Endl;
		}
	}

	return variable;
}

GlslVariable* GlslContext::emitInput(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT (inputPin);

	return emitInput(inputPin);
}

GlslVariable* GlslContext::emitOutput(Node* node, const std::wstring& outputPinName, GlslType type)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT (outputPin);

	GlslVariable* out = m_currentShader->createTemporaryVariable(outputPin, type);
	T_ASSERT (out);

	return out;
}

void GlslContext::emitOutput(Node* node, const std::wstring& outputPinName, GlslVariable* variable)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT (outputPin);

	m_currentShader->associateVariable(outputPin, variable);
}

void GlslContext::findNonDependentOutputs(Node* node, const std::wstring& inputPinName, const std::vector< const OutputPin* >& dependentOutputPins, std::vector< const OutputPin* >& outOutputPins) const
{
	getNonDependentOutputs(m_shaderGraph, node->findInputPin(inputPinName), dependentOutputPins, outOutputPins);
}

void GlslContext::findCommonOutputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, std::vector< const OutputPin* >& outOutputPins) const
{
	std::vector< const InputPin* > inputPins(2);
	inputPins[0] = node->findInputPin(inputPin1);
	inputPins[1] = node->findInputPin(inputPin2);
	getMergingOutputs(m_shaderGraph, inputPins, outOutputPins); 
}

void GlslContext::enterVertex()
{
	m_currentShader = &m_vertexShader;
}

void GlslContext::enterFragment()
{
	m_currentShader = &m_fragmentShader;
}

bool GlslContext::inVertex() const
{
	return bool(m_currentShader == &m_vertexShader);
}

bool GlslContext::inFragment() const
{
	return bool(m_currentShader == &m_fragmentShader);
}

bool GlslContext::allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset)
{
	int32_t lastId = int32_t(m_interpolatorMap.size());

	for (int32_t i = 0; i < lastId; ++i)
	{
		uint8_t& occupied = m_interpolatorMap[i];
		if (width <= 4 - occupied)
		{
			outId = i;
			outOffset = occupied;

			occupied += width;

			return false;
		}
	}

	outId = lastId;
	outOffset = 0;

	m_interpolatorMap.push_back(width);
	return true;
}

void GlslContext::defineParameter(const std::wstring& name, ParameterType type, int32_t length, UpdateFrequency frequency)
{
	if (getParameter(name) != 0)
		return;
	m_parameters.push_back({
		name,
		type,
		length,
		frequency
	});
}

const GlslContext::Parameter* GlslContext::getParameter(const std::wstring& name) const
{
	for (std::vector< Parameter >::const_iterator i = m_parameters.begin(); i != m_parameters.end(); ++i)
	{
		if (i->name == name)
			return &(*i);
	}
	return 0;
}

void GlslContext::setRenderState(const RenderState& renderState)
{
	m_renderState = renderState;
}

	}
}
