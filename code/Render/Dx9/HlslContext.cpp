#include <sstream>
#include "Render/Dx9/HlslContext.h"
#include "Render/Dx9/HlslShader.h"
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

HlslContext::HlslContext(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
,	m_shaderGraphAdj(new ShaderGraphAdjacency(shaderGraph))
,	m_vertexShader(HlslShader::StVertex)
,	m_pixelShader(HlslShader::StPixel)
,	m_currentShader(0)
,	m_interpolatorCount(0)
,	m_booleanRegisterCount(0)
,	m_needVPos(false)
{
}

HlslVariable* HlslContext::emitInput(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraphAdj->findSourcePin(inputPin);
	if (!sourcePin)
		return 0;

	HlslVariable* variable = m_currentShader->getVariable(sourcePin);
	if (!variable)
	{
		m_emitter.emit(*this, sourcePin->getNode());
		variable = m_currentShader->getVariable(sourcePin);
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

int32_t HlslContext::allocateInterpolator()
{
	return m_interpolatorCount++;
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
