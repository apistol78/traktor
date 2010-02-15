#include "Render/Shader/Node.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/ShaderGraphTypeEvaluator.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphTypeEvaluator", ShaderGraphTypeEvaluator, Object)

ShaderGraphTypeEvaluator::ShaderGraphTypeEvaluator(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
}

PinType ShaderGraphTypeEvaluator::evaluate(const InputPin* inputPin) const
{
	const OutputPin* outputPin = m_shaderGraph->findSourcePin(inputPin);
	if (outputPin)
		return evaluate(outputPin);
	else
		return PntVoid;
}

PinType ShaderGraphTypeEvaluator::evaluate(const Node* node, const std::wstring& inputPinName) const
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	if (inputPin)
		return evaluate(inputPin);
	else
		return PntVoid;
}

PinType ShaderGraphTypeEvaluator::evaluate(const OutputPin* outputPin) const
{
	std::map< const OutputPin*, PinType >::const_iterator i = m_cache.find(outputPin);
	if (i != m_cache.end())
		return i->second;

	m_cache[outputPin] = PntVoid;

	const Node* node = outputPin->getNode();
	T_ASSERT (node);

	const INodeTraits* nodeTraits = findNodeTraits(node);
	T_ASSERT (nodeTraits);

	uint32_t inputPinCount = node->getInputPinCount();

	// Evaluate input pin types.
	std::vector< PinType > inputPinTypes(inputPinCount);
	for (uint32_t i = 0; i < inputPinCount; ++i)
		inputPinTypes[i] = evaluate(node->getInputPin(i));

	// Determine output pin type from trait.
	PinType outputPinType = nodeTraits->getOutputPinType(
		node,
		outputPin,
		inputPinCount > 0 ? &inputPinTypes[0] : 0
	);

	m_cache[outputPin] = outputPinType;
	return outputPinType;
}

	}
}
