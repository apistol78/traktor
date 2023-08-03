/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphOrderEvaluator.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphOrderEvaluator", ShaderGraphOrderEvaluator, Object)

ShaderGraphOrderEvaluator::ShaderGraphOrderEvaluator(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
}

PinOrder ShaderGraphOrderEvaluator::evaluate(const Node* node, const std::wstring& inputPinName) const
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT(inputPin);

	const OutputPin* sourceOutputPin = m_shaderGraph->findSourcePin(inputPin);
	if (sourceOutputPin)
		return evaluate(sourceOutputPin);
	else
		return PinOrder::Constant;
}

PinOrder ShaderGraphOrderEvaluator::evaluate(const OutputPin* outputPin) const
{
	auto it = m_evaluated.find(outputPin);
	if (it != m_evaluated.end())
		return it->second;

	// Initialize entry to prevent infinite recursion; if we
	// detect recursion we assume high order.
	m_evaluated[outputPin] = PinOrder::NonLinear;

	const Node* node = outputPin->getNode();
	T_ASSERT(node);

	const int32_t inputPinCount = node->getInputPinCount();

	// Evaluate order of input pins.
	PinOrder inputPinOrders[32];
	T_ASSERT(inputPinCount < sizeof_array(inputPinOrders));

	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		const InputPin* inputPin = node->getInputPin(i);
		T_ASSERT(inputPin);

		const OutputPin* sourceOutputPin = m_shaderGraph->findSourcePin(inputPin);
		if (sourceOutputPin)
			inputPinOrders[i] = evaluate(sourceOutputPin);
		else
			inputPinOrders[i] = PinOrder::Constant;
	}

	PinOrder order = PinOrder::NonLinear;

	// Use node's traits to determine order of queried output pin.
	const INodeTraits* nodeTraits = INodeTraits::find(node);
	if (nodeTraits)
	{
		order = nodeTraits->evaluateOrder(
			m_shaderGraph,
			node,
			outputPin,
			inputPinCount > 0 ? inputPinOrders : nullptr
		);
	}

	m_evaluated[outputPin] = order;
	return order;
}

}
