#include <sstream>
#include "Render/Ps3/CgContext.h"
#include "Render/Ps3/CgShader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/OutputPin.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace render
	{

CgContext::CgContext(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
,	m_vertexShader(CgShader::StVertex)
,	m_pixelShader(CgShader::StPixel)
,	m_currentShader(0)
,	m_booleanRegisterCount(0)
,	m_needVPos(false)
,	m_registerCount(0)
{
}

CgVariable* CgContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	CgVariable* variable = m_currentShader->getVariable(sourcePin);
	if (!variable)
	{
		if (m_emitter.emit(*this, sourcePin->getNode()))
		{
			variable = m_currentShader->getVariable(sourcePin);
			T_ASSERT (variable);
		}
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

void CgContext::emitOutput(Node* node, const std::wstring& outputPinName, CgVariable* variable)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT (outputPin);

	m_currentShader->associateVariable(outputPin, variable);
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

bool CgContext::allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset)
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

int32_t CgContext::allocateBooleanRegister()
{
	return m_booleanRegisterCount++;
}

void CgContext::allocateVPos()
{
	m_needVPos = true;
}

void CgContext::setRegisterCount(uint32_t registerCount)
{
	m_registerCount = registerCount;
}

	}
}
