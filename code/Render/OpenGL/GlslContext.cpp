#include <sstream>
#include "Render/OpenGL/GlslContext.h"
#include "Render/OpenGL/GlslShader.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{

GlslContext::GlslContext(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
,	m_vertexShader(GlslShader::StVertex)
,	m_fragmentShader(GlslShader::StFragment)
,	m_currentShader(0)
,	m_nextStage(0)
,	m_requireDerivatives(false)
,	m_requireTranspose(false)
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

GlslVariable* GlslContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	GlslVariable* variable = m_currentShader->getVariable(sourcePin);
	if (!variable)
	{
		if (m_emitter.emit(*this, sourcePin->getNode()))
		{
			variable = m_currentShader->getVariable(sourcePin);
			T_ASSERT (variable);
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

bool GlslContext::isPinsConnected(const OutputPin* outputPin, const InputPin* inputPin) const
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

void GlslContext::findExternalInputs(Node* node, const std::wstring& inputPinName, const std::wstring& dependentOutputPinName, std::vector< const InputPin* >& outInputPins) const
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

void GlslContext::setRequireDerivatives()
{
	m_requireDerivatives = true;
}
	
bool GlslContext::getRequireDerivatives() const
{
	return m_requireDerivatives;
}

void GlslContext::setRequireTranspose()
{
	m_requireTranspose = true;
}

bool GlslContext::getRequireTranspose() const
{
	return m_requireTranspose;
}

GlslShader& GlslContext::getVertexShader()
{
	return m_vertexShader;
}

GlslShader& GlslContext::getFragmentShader()
{
	return m_fragmentShader;
}

GlslShader& GlslContext::getShader()
{
	T_ASSERT (m_currentShader);
	return *m_currentShader;
}

GlslEmitter& GlslContext::getEmitter()
{
	return m_emitter;
}

RenderStateOpenGL& GlslContext::getRenderState()
{
	return m_renderState;
}

void GlslContext::defineTexture(const std::wstring& texture)
{
	if (std::find(m_textures.begin(), m_textures.end(), texture) == m_textures.end())
		m_textures.push_back(texture);
}

bool GlslContext::defineSampler(uint32_t stateHash, GLenum target, const std::wstring& texture, int32_t& outStage)
{
	outStage = m_nextStage++;

	// Create sampler binding.
	std::vector< std::wstring >::iterator i = std::find(m_textures.begin(), m_textures.end(), texture);
	T_ASSERT (i != m_textures.end());

	SamplerBindingOpenGL sb;
	sb.stage = outStage;
	sb.target = target;
	sb.texture = std::distance(m_textures.begin(), i);
	m_samplers.push_back(sb);

	return true;
}

const std::vector< std::wstring >& GlslContext::getTextures() const
{
	return m_textures;
}

const std::vector< SamplerBindingOpenGL >& GlslContext::getSamplers() const
{
	return m_samplers;
}

	}
}
