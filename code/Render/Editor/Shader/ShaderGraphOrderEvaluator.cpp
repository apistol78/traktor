/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/ShaderGraphOrderEvaluator.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphOrderEvaluator", ShaderGraphOrderEvaluator, Object)

ShaderGraphOrderEvaluator::ShaderGraphOrderEvaluator(
	const ShaderGraph* shaderGraph,
	bool frequentUniformsAsLinear
)
:	m_shaderGraph(shaderGraph)
,	m_frequentUniformsAsLinear(frequentUniformsAsLinear)
{
}

PinOrderType ShaderGraphOrderEvaluator::evaluate(const Node* node, const std::wstring& inputPinName) const
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT (inputPin);

	const OutputPin* sourceOutputPin = m_shaderGraph->findSourcePin(inputPin);
	if (sourceOutputPin)
		return evaluate(sourceOutputPin);
	else
		return PotConstant;
}

PinOrderType ShaderGraphOrderEvaluator::evaluate(const OutputPin* outputPin) const
{
	std::map< const OutputPin*, PinOrderType >::iterator i = m_evaluated.find(outputPin);
	if (i != m_evaluated.end())
		return i->second;

	// Initialize entry to prevent infinite recursion; if we
	// detect recursion we assume high order.
	m_evaluated[outputPin] = PotNonLinear;

	const Node* node = outputPin->getNode();
	T_ASSERT (node);

	int32_t inputPinCount = node->getInputPinCount();

	// Evaluate order of input pins.
	PinOrderType inputPinOrders[32];
	T_ASSERT (inputPinCount < sizeof_array(inputPinOrders));

	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		const InputPin* inputPin = node->getInputPin(i);
		T_ASSERT (inputPin);

		const OutputPin* sourceOutputPin = m_shaderGraph->findSourcePin(inputPin);
		if (sourceOutputPin)
			inputPinOrders[i] = evaluate(sourceOutputPin);
		else
			inputPinOrders[i] = PotConstant;
	}

	PinOrderType order = PotNonLinear;

	// Use node's traits to determine order of queried output pin.
	const INodeTraits* nodeTraits = INodeTraits::find(node);
	if (nodeTraits)
	{
		order = nodeTraits->evaluateOrder(
			m_shaderGraph,
			node,
			outputPin,
			inputPinCount > 0 ? inputPinOrders : 0,
			m_frequentUniformsAsLinear
		);
	}

	m_evaluated[outputPin] = order;
	return order;
}

	}
}
