/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/MatrixNodeTraits.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

int32_t getInputPinIndex(const Node* node, const InputPin* inputPin)
{
	const int32_t inputPinCount = node->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		if (node->getInputPin(i) == inputPin)
			return i;
	}
	T_FATAL_ERROR;
	return -1;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MatrixNodeTraits", 0, MatrixNodeTraits, INodeTraits)

TypeInfoSet MatrixNodeTraits::getNodeTypes() const
{
	return makeTypeInfoSet<
		MatrixIn,
		MatrixOut
	>();
}

bool MatrixNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

bool MatrixNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType
) const
{
	if (is_a< MatrixIn >(node))
		return isPinTypeScalar(pinType);
	else if (is_a< MatrixOut >(node))
		return pinType == PinType::Matrix;
	else
		return false;
}

PinType MatrixNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (is_a< MatrixIn >(node))
		return PinType::Matrix;
	else if (is_a< MatrixOut >(node))
		return PinType::Scalar4;
	else
		return PinType::Void;
}

PinType MatrixNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (is_a< MatrixIn >(node))
		return PinType::Scalar4;
	else if (is_a< MatrixOut >(node))
		return PinType::Matrix;
	else
		return PinType::Void;
}

int32_t MatrixNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool MatrixNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool MatrixNodeTraits::evaluatePartial(
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

PinOrder MatrixNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders
) const
{
	if (is_a< MatrixIn >(node))
		return pinOrderConstantOrNonLinear(inputPinOrders, node->getInputPinCount());
	else if (is_a< MatrixOut >(node))
		return pinOrderMax(inputPinOrders, node->getInputPinCount());
	else
		return PinOrder::Constant;
}

	}
}
