/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphTypeEvaluator.h"

namespace traktor::render
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
		return PinType::Void;
}

PinType ShaderGraphTypeEvaluator::evaluate(const Node* node, const std::wstring& inputPinName) const
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	if (inputPin)
		return evaluate(inputPin);
	else
		return PinType::Void;
}

PinType ShaderGraphTypeEvaluator::evaluate(const OutputPin* outputPin) const
{
	auto i = m_cache.find(outputPin);
	if (i != m_cache.end())
		return i->second;

	PinType outputPinType = PinType::Void;
	m_cache[outputPin] = PinType::Void;

	const Node* node = outputPin->getNode();
	T_ASSERT(node);

	if (is_a< Type >(node))
	{
		const PinType inputType = evaluate(node, L"Type");

		const InputPin* inputPin = nullptr;
		if (isPinTypeScalar(inputType))
		{
			if (getPinTypeWidth(inputType) <= 1)
				inputPin = node->findInputPin(L"Scalar");
			else
				inputPin = node->findInputPin(L"Vector");
		}
		else if (inputType == PinType::Matrix)
		{
			inputPin = node->findInputPin(L"Matrix");
		}
		else if (isPinTypeTexture(inputType))
		{
			inputPin = node->findInputPin(L"Texture");
		}
		if (!inputPin)
			return PinType::Void;

		outputPinType = evaluate(inputPin);
	}
	else
	{
		const INodeTraits* nodeTraits = INodeTraits::find(node);
		T_ASSERT(nodeTraits);

		const uint32_t inputPinCount = node->getInputPinCount();

		// Evaluate input pin types.
		AlignedVector< PinType > inputPinTypes(inputPinCount);
		for (uint32_t i = 0; i < inputPinCount; ++i)
			inputPinTypes[i] = evaluate(node->getInputPin(i));

		// Determine output pin type from trait.
		outputPinType = nodeTraits->getOutputPinType(
			m_shaderGraph,
			node,
			outputPin,
			inputPinCount > 0 ? &inputPinTypes[0] : nullptr
		);
	}

	m_cache[outputPin] = outputPinType;
	return outputPinType;
}

}
