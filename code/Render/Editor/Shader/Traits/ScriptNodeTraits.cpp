/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Traits/ScriptNodeTraits.h"

#include "Render/Editor/Shader/Script.h"

#include <cctype>

namespace traktor::render
{
namespace
{

int32_t getInputPinIndex(const Node* node, const InputPin* inputPin)
{
	const int32_t inputPinCount = node->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
		if (node->getInputPin(i) == inputPin)
			return i;
	T_FATAL_ERROR;
	return -1;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ScriptNodeTraits", 0, ScriptNodeTraits, INodeTraits)

TypeInfoSet ScriptNodeTraits::getNodeTypes() const
{
	return makeTypeInfoSet< Script >();
}

bool ScriptNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	const Script* script = static_cast< const Script* >(node);
	return !script->getTechnique().empty();
}

bool ScriptNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType) const
{
	return true;
}

PinType ScriptNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes) const
{
	T_ASSERT(is_a< Script >(node));
	T_ASSERT(outputPin->getNode() == node);

	const TypedOutputPin* typedOutputPin = static_cast< const TypedOutputPin* >(outputPin);
	switch (typedOutputPin->getType())
	{
	case ParameterType::Scalar:
		return PinType::Scalar1;
	case ParameterType::Vector:
		return PinType::Scalar4;
	case ParameterType::Matrix:
		return PinType::Matrix;
	case ParameterType::Texture2D:
		return PinType::Texture2D;
	case ParameterType::Texture3D:
		return PinType::Texture3D;
	case ParameterType::TextureCube:
		return PinType::TextureCube;
	default:
		return PinType::Void;
	}
}

PinType ScriptNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes) const
{
	for (int32_t i = 0; i < node->getInputPinCount(); ++i)
		if (node->getInputPin(i) == inputPin)
			return inputPinTypes[i];
	return PinType::Void;
}

int32_t ScriptNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin) const
{
	return getInputPinIndex(node, inputPin);
}

bool ScriptNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant) const
{
	return false;
}

bool ScriptNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin) const
{
	return false;
}

PinOrder ScriptNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders) const
{
	return PinOrder::NonLinear;
}

}
