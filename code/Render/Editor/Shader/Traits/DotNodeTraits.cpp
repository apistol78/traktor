/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/DotNodeTraits.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.DotNodeTraits", 0, DotNodeTraits, INodeTraits)

TypeInfoSet DotNodeTraits::getNodeTypes() const
{
	return makeTypeInfoSet< Dot >();
}

bool DotNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

bool DotNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType
) const
{
	return isPinTypeScalar(pinType);
}

PinType DotNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return PinType::Scalar1;
}

PinType DotNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	return PinType::Scalar4;
}

int32_t DotNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return 0;
}

bool DotNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	const int32_t w0 = inputConstants[0].getWidth();
	const int32_t w1 = inputConstants[1].getWidth();

	if (w0 > 0 && w0 == w1)
	{
		if (inputConstants[0].isAllConst() && inputConstants[1].isAllConst())
		{
			outputConstant = Constant(
				inputConstants[0].x() * inputConstants[1].x() +
				inputConstants[0].y() * inputConstants[1].y() +
				inputConstants[0].z() * inputConstants[1].z() +
				inputConstants[0].w() * inputConstants[1].w()
			);
		}
		else if (inputConstants[0].isAllZero() || inputConstants[1].isAllZero())
		{
			outputConstant = Constant(0.0f);
			return true;
		}
	}
	
	return false;
}

bool DotNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin
) const
{
	return false;
}

PinOrder DotNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders
) const
{
	return pinOrderConstantOrNonLinear(inputPinOrders, node->getInputPinCount());
}

}
