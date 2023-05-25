/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/SmallMap.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/GraphTraverse.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphEvaluator.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphEvaluator", ShaderGraphEvaluator, Object)

ShaderGraphEvaluator::ShaderGraphEvaluator(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
}

Constant ShaderGraphEvaluator::evaluate(const OutputPin* outputPin) const
{
	SmallMap< const OutputPin*, Constant > evaluatedConstants;
	AlignedVector< PinType > inputPinTypes;
	AlignedVector< Constant > inputConstants;

	GraphTraverse(m_shaderGraph, outputPin->getNode()).postorder([&, this](const Node* node) {
		const INodeTraits* nodeTraits = INodeTraits::find(node);
		T_ASSERT(nodeTraits);

		// Get node's input types and constants.
		const int32_t inputPinCount = node->getInputPinCount();
		inputPinTypes.resize(inputPinCount);
		inputConstants.resize(inputPinCount);

		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			const OutputPin* outputPin = m_shaderGraph->findSourcePin(inputPin);
			if (outputPin)
			{
				inputPinTypes[i] = evaluatedConstants[outputPin].getType();
				inputConstants[i] = evaluatedConstants[outputPin];
			}
			else
			{
				inputPinTypes[i] = PinType::Void;
				inputConstants[i] = Constant();
			}
		}

		// Evaluate output constants from input set.
		const int32_t outputPinCount = node->getOutputPinCount();
		for (int32_t i = 0; i < outputPinCount; ++i)
		{
			const OutputPin* outputPin = node->getOutputPin(i);

			const PinType outputPinType = nodeTraits->getOutputPinType(
				m_shaderGraph,
				node,
				outputPin,
				inputPinTypes.c_ptr()
			);

			evaluatedConstants[outputPin] = Constant(outputPinType);

			nodeTraits->evaluatePartial(
				m_shaderGraph,
				node,
				outputPin,
				inputConstants.c_ptr(),
				evaluatedConstants[outputPin]
			);
		}
		return true;
	});

	return evaluatedConstants[outputPin];
}

	}
}
