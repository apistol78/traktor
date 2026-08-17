/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Traits/MetaNodeTraits.h"

#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"

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

/*! Input pins of the IsConstant node. */
struct IsConstantPin
{
	enum
	{
		Input = 0,
		True = 1,
		False = 2
	};
};

/*! Check if "Input" pin of an IsConstant node is a known constant value.
 *
 * Only connected, entirely constant, scalar inputs are considered constant;
 * an unconnected input is never constant as it doesn't carry a value at all.
 */
bool isConstantInput(const ShaderGraph* shaderGraph, const Node* node, const Constant* inputConstants)
{
	if (shaderGraph->findSourcePin(node->getInputPin(IsConstantPin::Input)) != nullptr)
		return inputConstants[IsConstantPin::Input].getWidth() > 0 && inputConstants[IsConstantPin::Input].isAllConst();
	else
		return false;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MetaNodeTraits", 0, MetaNodeTraits, INodeTraits)

TypeInfoSet MetaNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< Branch >();
	typeSet.insert< BundleUnite >();
	typeSet.insert< BundleSplit >();
	typeSet.insert< Comment >();
	typeSet.insert< Connected >();
	typeSet.insert< External >();
	typeSet.insert< InputPort >();
	typeSet.insert< IsConstant >();
	typeSet.insert< OutputPort >();
	typeSet.insert< Platform >();
	typeSet.insert< PreviewInput >();
	typeSet.insert< PreviewOutput >();
	typeSet.insert< Renderer >();
	typeSet.insert< Type >();
	typeSet.insert< Variable >();
	return typeSet;
}

bool MetaNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	if (is_a< InputPort >(node) || is_a< OutputPort >(node))
		return true;
	else if (const External* externalNode = dynamic_type_cast< const External* >(node))
		return externalNode->getOutputPinCount() == 0;
	else if (const Variable* variableNode = dynamic_type_cast< const Variable* >(node))
		return bool(shaderGraph->findSourcePin(variableNode->getInputPin(0)) != nullptr) &&
			bool(shaderGraph->getDestinationCount(variableNode->getOutputPin(0)) == 0);
	else
		return false;
}

bool MetaNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType) const
{
	return true;
}

PinType MetaNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes) const
{
	PinType outputPinType = PinType::Void;

	// Note; IsConstant include it's probed "Input" pin in the estimate as well, since
	// without any path connected the node still output a scalar "is constant" flag.
	if (is_a< Branch >(node) || is_a< Connected >(node) || is_a< IsConstant >(node) || is_a< Platform >(node) || is_a< Renderer >(node))
	{
		const uint32_t inputPinCount = node->getInputPinCount();
		for (uint32_t i = 0; i < inputPinCount; ++i)
			outputPinType = std::max< PinType >(
				outputPinType,
				inputPinTypes[i]);
	}
	else if (is_a< BundleUnite >(node))
		return PinType::Bundle;
	else if (is_a< Type >(node))
	{
		switch (inputPinTypes[0])
		{
		default:
		case PinType::Void:
			break;

		case PinType::Scalar1:
			outputPinType = inputPinTypes[1];
			break;

		case PinType::Scalar2:
		case PinType::Scalar3:
		case PinType::Scalar4:
			outputPinType = inputPinTypes[2];
			break;

		case PinType::Matrix:
			outputPinType = inputPinTypes[3];
			break;

		case PinType::Texture2D:
		case PinType::Texture3D:
		case PinType::TextureCube:
			outputPinType = inputPinTypes[4];
			break;

		case PinType::StructBuffer:
			break;

		case PinType::State:
			outputPinType = inputPinTypes[5];
			break;
		}
	}
	else if (is_a< PreviewInput >(node))
		outputPinType = PinType::Scalar4;
	return outputPinType;
}

PinType MetaNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes) const
{
	if (is_a< OutputPort >(node))
		return PinType::Void;
	else if (is_a< IsConstant >(node) && getInputPinIndex(node, inputPin) == IsConstantPin::Input)
		return PinType::Void;
	else
		return outputPinTypes[0];
}

int32_t MetaNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin) const
{
	return getInputPinIndex(node, inputPin);
}

bool MetaNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant) const
{
	if (is_a< IsConstant >(node))
	{
		if (!isConstantInput(shaderGraph, node, inputConstants))
			return false;

		if (shaderGraph->findSourcePin(node->getInputPin(IsConstantPin::True)) != nullptr)
			outputConstant = inputConstants[IsConstantPin::True];
		else
		{
			// No "True" path connected; node is used as a plain "is constant" query.
			for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
				outputConstant.setValue(i, 1.0f);
		}

		return outputConstant.getWidth() > 0;
	}
	return false;
}

bool MetaNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin) const
{
	if (is_a< IsConstant >(node))
	{
		if (!isConstantInput(shaderGraph, node, inputConstants))
			return false;
		if (inputOutputPins[IsConstantPin::True] == nullptr)
			return false;
		foldOutputPin = inputOutputPins[IsConstantPin::True];
		return true;
	}
	return false;
}

PinOrder MetaNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders) const
{
	if (is_a< IsConstant >(node))
		return pinOrderMax(&inputPinOrders[IsConstantPin::True], 2);
	else
		return pinOrderMax(inputPinOrders, node->getInputPinCount());
}

}
