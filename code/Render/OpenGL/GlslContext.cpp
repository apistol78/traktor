#include <sstream>
#include "Render/OpenGL/GlslContext.h"
#include "Render/OpenGL/GlslShader.h"
#include "Render/ShaderGraph.h"
#include "Render/ShaderGraphAdjacency.h"
#include "Render/Node.h"
#include "Render/InputPin.h"
#include "Render/OutputPin.h"

namespace traktor
{
	namespace render
	{

GlslContext::GlslContext(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
,	m_shaderGraphAdj(new ShaderGraphAdjacency(shaderGraph))
,	m_vertexShader(GlslShader::StVertex)
,	m_fragmentShader(GlslShader::StFragment)
,	m_currentShader(0)
{
}

GlslVariable* GlslContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraphAdj->findSourcePin(inputPin);
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

	}
}
