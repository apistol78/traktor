#include <sstream>
#include "Render/OpenGL/GlslContext.h"
#include "Render/OpenGL/GlslShader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/OutputPin.h"

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

GlslVariable* GlslContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	GlslVariable* variable = m_currentShader->getVariable(sourcePin);
	if (!variable)
	{
		m_emitter.emit(*this, sourcePin->getNode());
		variable = m_currentShader->getVariable(sourcePin);
		T_ASSERT (variable);
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

RenderState& GlslContext::getRenderState()
{
	return m_renderState;
}

bool GlslContext::defineSamplerTexture(const std::wstring& textureName, int32_t& outStage)
{
	std::map< std::wstring, int32_t >::iterator i = m_samplerTextures.find(textureName);
	if (i != m_samplerTextures.end())
	{
		outStage = i->second;
		return false;
	}

	outStage = m_nextStage++;
	m_samplerTextures.insert(std::make_pair(textureName, outStage));

	return true;
}

const std::map< std::wstring, int32_t >& GlslContext::getSamplerTextures() const
{
	return m_samplerTextures;
}

	}
}
