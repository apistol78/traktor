/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/ComputeNodeTraits.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ComputeNodeTraits", 0, ComputeNodeTraits, INodeTraits)

TypeInfoSet ComputeNodeTraits::getNodeTypes() const
{
	return makeTypeInfoSet< ComputeOutput >();
}

bool ComputeNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return is_a< ComputeOutput >(node);
}

bool ComputeNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType
) const
{
	if (inputPin->getName() == L"Storage")
		return isPinTypeStructBuffer(pinType) || isPinTypeTexture(pinType) || isPinTypeImage(pinType);
	else
		return isPinTypeScalar(pinType);
}

PinType ComputeNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return PinType::Void;
}

PinType ComputeNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Enable")
		return PinType::Scalar1;
	else if (inputPin->getName() == L"Storage")
		return PinType::Any;
	else if (inputPin->getName() == L"Offset")
		return (inputPinTypes[1] == PinType::Texture2D || inputPinTypes[1] == PinType::Image2D) ? PinType::Scalar2 : PinType::Scalar3;
	else
		return PinType::Scalar4;
}

int32_t ComputeNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return 0;
}

bool ComputeNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool ComputeNodeTraits::evaluatePartial(
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

PinOrder ComputeNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders
) const
{
	return PinOrder::Linear;
}

}
