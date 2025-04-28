/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Traits/TransposeNodeTraits.h"

#include "Render/Editor/Shader/Nodes.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TransposeNodeTraits", 0, TransposeNodeTraits, INodeTraits)

TypeInfoSet TransposeNodeTraits::getNodeTypes() const
{
	return makeTypeInfoSet< Transpose >();
}

bool TransposeNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

bool TransposeNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType) const
{
	return pinType == PinType::Matrix;
}

PinType TransposeNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes) const
{
	return PinType::Matrix;
}

PinType TransposeNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes) const
{
	return PinType::Matrix;
}

int32_t TransposeNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin) const
{
	return 0;
}

bool TransposeNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant) const
{
	return false;
}

bool TransposeNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin) const
{
	return false;
}

PinOrder TransposeNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders) const
{
	return inputPinOrders[0];
}

}
