/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Traits/ArrayNodeTraits.h"

#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ArrayNodeTraits", 0, ArrayNodeTraits, INodeTraits)

TypeInfoSet ArrayNodeTraits::getNodeTypes() const
{
	return makeTypeInfoSet< ArrayElement, ArrayLength >();
}

bool ArrayNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

bool ArrayNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType) const
{
	if (const ArrayElement* arrayElement = dynamic_type_cast< const ArrayElement* >(node))
	{
		if (inputPin->getName() == L"Index")
			return isPinTypeScalar(pinType);
		else
			return isPinTypeArray(pinType) || isPinTypeStructBuffer(pinType);
	}
	else if (const ArrayLength* arrayLength = dynamic_type_cast< const ArrayLength* >(node))
	{
		return isPinTypeArray(pinType);
	}
	else
		return false;
}

PinType ArrayNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes) const
{
	if (const ArrayElement* arrayElement = dynamic_type_cast< const ArrayElement* >(node))
	{
		const OutputPin* prmOutputPin = shaderGraph->findSourcePin(arrayElement->findInputPin(L"Input"));
		if (!prmOutputPin)
			return PinType::Void;

		const Parameter* prm = dynamic_type_cast< const Parameter* >(prmOutputPin->getNode());
		if (!prm)
			return PinType::Void;

		switch (prm->getDeclaration().getParameterType())
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
		case ParameterType::Image2D:
			return PinType::Image2D;
		case ParameterType::Image3D:
			return PinType::Image3D;
		case ParameterType::ImageCube:
			return PinType::ImageCube;
		case ParameterType::StructBuffer:
			return PinType::Struct;
		default:
			return PinType::Void;
		}
	}
	else if (const ArrayLength* arrayLength = dynamic_type_cast< const ArrayLength* >(node))
	{
		return PinType::Scalar1;
	}
	return PinType::Void;
}

PinType ArrayNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes) const
{
	return PinType::Array;
}

int32_t ArrayNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin) const
{
	return 0;
}

bool ArrayNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant) const
{
	if (const ArrayLength* arrayLength = dynamic_type_cast< const ArrayLength* >(node))
	{
		const OutputPin* prmOutputPin = shaderGraph->findSourcePin(arrayLength->findInputPin(L"Input"));
		if (!prmOutputPin)
			return false;

		const Parameter* prm = dynamic_type_cast< const Parameter* >(prmOutputPin->getNode());
		if (!prm)
			return false;

		outputConstant = Constant((float)prm->getDeclaration().getLength());
		return true;
	}
	return false;
}

bool ArrayNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin) const
{
	return false;
}

PinOrder ArrayNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders) const
{
	return pinOrderConstantOrNonLinear(inputPinOrders, node->getInputPinCount());
}

}
