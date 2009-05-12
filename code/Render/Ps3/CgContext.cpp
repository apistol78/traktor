#include <cassert>
#include <sstream>
#include "Render/Ps3/CgContext.h"
#include "Render/Ps3/CgShader.h"
#include "Render/ShaderGraph.h"
#include "Render/Node.h"
#include "Render/InputPin.h"
#include "Render/OutputPin.h"
#include "Core/Heap/Ref.h"

namespace traktor
{
	namespace render
	{

CgContext::CgContext(ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
,	m_vertexShader(CgShader::StVertex)
,	m_pixelShader(CgShader::StPixel)
,	m_currentShader(0)
{
}

CgVariable* CgContext::emitInput(Node* node, const std::wstring& inputPinName)
{
	Ref< InputPin > inputPin = node->findInputPin(inputPinName);
	assert (inputPin);

	Ref< OutputPin > sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	CgVariable* variable = m_currentShader->getVariable(sourcePin);
	if (!variable)
	{
		m_emitter.emit(*this, sourcePin->getNode());
		variable = m_currentShader->getVariable(sourcePin);
		assert (variable);
	}

	return variable;
}

CgVariable* CgContext::emitOutput(Node* node, const std::wstring& outputPinName, CgType type)
{
	Ref< OutputPin > outputPin = node->findOutputPin(outputPinName);
	assert (outputPin);

	CgVariable* out = m_currentShader->createTemporaryVariable(outputPin, type);
	assert (out);

	return out;
}

void CgContext::enterVertex()
{
	m_currentShader = &m_vertexShader;
}

void CgContext::enterPixel()
{
	m_currentShader = &m_pixelShader;
}

bool CgContext::inVertex() const
{
	return bool(m_currentShader == &m_vertexShader);
}

bool CgContext::inPixel() const
{
	return bool(m_currentShader == &m_pixelShader);
}

CgShader& CgContext::getVertexShader()
{
	return m_vertexShader;
}

CgShader& CgContext::getPixelShader()
{
	return m_pixelShader;
}

CgShader& CgContext::getShader()
{
	assert (m_currentShader);
	return *m_currentShader;
}

CgEmitter& CgContext::getEmitter()
{
	return m_emitter;
}

RenderState& CgContext::getRenderState()
{
	return m_renderState;
}

	}
}
