#include <sstream>
#include "Render/Ps3/CgContext.h"
#include "Render/Ps3/CgShader.h"
#include "Render/ShaderGraph.h"
#include "Render/ShaderGraphAdjacency.h"
#include "Render/Node.h"
#include "Render/InputPin.h"
#include "Render/OutputPin.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace render
	{

CgContext::CgContext(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
,	m_shaderGraphAdj(new ShaderGraphAdjacency(shaderGraph))
,	m_vertexShader(CgShader::StVertex)
,	m_pixelShader(CgShader::StPixel)
,	m_currentShader(0)
,	m_interpolatorCount(0)
,	m_booleanRegisterCount(0)
,	m_needVPos(false)
{
}

CgVariable* CgContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraphAdj->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	CgVariable* variable = m_currentShader->getVariable(sourcePin);
	if (!variable)
	{
		m_emitter.emit(*this, sourcePin->getNode());
		variable = m_currentShader->getVariable(sourcePin);
	}

	T_ASSERT (!variable || variable->getType() != CtVoid);
	return variable;
}

CgVariable* CgContext::emitInput(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT (inputPin);

	return emitInput(inputPin);
}

CgVariable* CgContext::emitOutput(Node* node, const std::wstring& outputPinName, CgType type)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT (outputPin);

	CgVariable* out = m_currentShader->createTemporaryVariable(outputPin, type);
	T_ASSERT (out);

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

int32_t CgContext::allocateInterpolator()
{
	return m_interpolatorCount++;
}

int32_t CgContext::allocateBooleanRegister()
{
	return m_booleanRegisterCount++;
}

void CgContext::allocateVPos()
{
	m_needVPos = true;
}

	}
}
