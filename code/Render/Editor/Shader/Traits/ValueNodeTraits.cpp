/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Traits/ValueNodeTraits.h"

#include "Render/Editor/Shader/Nodes.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ValueNodeTraits", 0, ValueNodeTraits, INodeTraits)

TypeInfoSet ValueNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< DispatchIndex >();
	typeSet.insert< FragmentPosition >();
	typeSet.insert< FrontFace >();
	return typeSet;
}

bool ValueNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

bool ValueNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType) const
{
	return false;
}

PinType ValueNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes) const
{
	if (is_a< DispatchIndex >(node))
		return PinType::Scalar3;
	else if (is_a< FragmentPosition >(node))
		return PinType::Scalar2;
	else if (is_a< FrontFace >(node))
		return PinType::Scalar1;
	else
		return PinType::Void;
}

PinType ValueNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes) const
{
	return PinType::Void;
}

int32_t ValueNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin) const
{
	return 0;
}

bool ValueNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant) const
{
	return false;
}

bool ValueNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin) const
{
	return false;
}

PinOrder ValueNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders) const
{
	return PinOrder::NonLinear;
}

}
