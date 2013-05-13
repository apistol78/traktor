#include <sstream>
#include "Core/Misc/String.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Dx9/HlslContext.h"
#include "Render/Dx9/HlslShader.h"

namespace traktor
{
	namespace render
	{

HlslContext::HlslContext(const ShaderGraph* shaderGraph, IProgramHints* programHints)
:	m_shaderGraph(shaderGraph)
,	m_vertexShader(HlslShader::StVertex, programHints)
,	m_pixelShader(HlslShader::StPixel, programHints)
,	m_currentShader(0)
,	m_booleanRegisterCount(0)
,	m_needVPos(false)
{
}

Node* HlslContext::getInputNode(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	return sourcePin ? sourcePin->getNode() : 0;
}

Node* HlslContext::getInputNode(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT (inputPin);

	return getInputNode(inputPin);
}

HlslVariable* HlslContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	HlslVariable* variable = m_currentShader->getVariable(sourcePin);
	if (!variable)
	{
		if (m_emitter.emit(*this, sourcePin->getNode()))
		{
			variable = m_currentShader->getVariable(sourcePin);
			T_ASSERT (variable);
		}
	}

	T_ASSERT (!variable || variable->getType() != HtVoid);
	return variable;
}

HlslVariable* HlslContext::emitInput(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT (inputPin);

	return emitInput(inputPin);
}

HlslVariable* HlslContext::emitOutput(Node* node, const std::wstring& outputPinName, HlslType type)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT (outputPin);

	HlslVariable* out = m_currentShader->createTemporaryVariable(outputPin, type);
	T_ASSERT (out);

	return out;
}

void HlslContext::emitOutput(Node* node, const std::wstring& outputPinName, HlslVariable* variable)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT (outputPin);

	m_currentShader->associateVariable(outputPin, variable);
}

bool HlslContext::isPinsConnected(const OutputPin* outputPin, const InputPin* inputPin) const
{
	const OutputPin* sourceOutputPin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourceOutputPin)
		return false;

	std::set< const OutputPin* > visitedOutputPins;
	visitedOutputPins.insert(sourceOutputPin);

	std::vector< const OutputPin* > outputPins;
	outputPins.push_back(sourceOutputPin);

	while (!outputPins.empty())
	{
		const OutputPin* sourceOutputPin = outputPins.back(); outputPins.pop_back();
		T_ASSERT (sourceOutputPin);

		if (sourceOutputPin == outputPin)
			return true;

		const Node* node = sourceOutputPin->getNode();
		for (int32_t i = 0; i < node->getInputPinCount(); ++i)
		{
			const InputPin* nodeInputPin = node->getInputPin(i);
			T_ASSERT (nodeInputPin);

			const OutputPin* nodeSourceOutputPin = m_shaderGraph->findSourcePin(nodeInputPin);
			if (
				nodeSourceOutputPin &&
				visitedOutputPins.find(nodeSourceOutputPin) == visitedOutputPins.end()
				)
			{
				outputPins.push_back(nodeSourceOutputPin);
				visitedOutputPins.insert(nodeSourceOutputPin);
			}
		}
	}

	return false;
}

void HlslContext::findExternalInputs(Node* node, const std::wstring& inputPinName, const std::wstring& dependentOutputPinName, std::vector< const InputPin* >& outInputPins) const
{
	const OutputPin* dependentOutputPin = node->findOutputPin(dependentOutputPinName);
	T_ASSERT (dependentOutputPin);

	std::set< const OutputPin* > visitedOutputPins;
	visitedOutputPins.insert(dependentOutputPin);

	std::vector< const InputPin* > inputPins;
	inputPins.push_back(node->findInputPin(inputPinName));

	while (!inputPins.empty())
	{
		const InputPin* inputPin = inputPins.back(); inputPins.pop_back();
		T_ASSERT (inputPin);

		if (!isPinsConnected(dependentOutputPin, inputPin))
			outInputPins.push_back(inputPin);
		else
		{
			const OutputPin* outputPin = m_shaderGraph->findSourcePin(inputPin);
			if (
				!outputPin ||
				visitedOutputPins.find(outputPin) != visitedOutputPins.end()
				)
				continue;

			visitedOutputPins.insert(outputPin);

			const Node* node = outputPin->getNode();
			for (int32_t i = 0; i < node->getInputPinCount(); ++i)
				inputPins.push_back(node->getInputPin(i));
		}
	}
}

void HlslContext::enterVertex()
{
	m_currentShader = &m_vertexShader;
}

void HlslContext::enterPixel()
{
	m_currentShader = &m_pixelShader;
}

bool HlslContext::inVertex() const
{
	return bool(m_currentShader == &m_vertexShader);
}

bool HlslContext::inPixel() const
{
	return bool(m_currentShader == &m_pixelShader);
}

bool HlslContext::allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset)
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

int32_t HlslContext::allocateBooleanRegister()
{
	return m_booleanRegisterCount++;
}

void HlslContext::allocateVPos()
{
	m_needVPos = true;
}

	}
}
