/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Traits/ParameterNodeTraits.h"

#include "Render/Editor/Shader/Nodes.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ParameterNodeTraits", 0, ParameterNodeTraits, INodeTraits)

TypeInfoSet ParameterNodeTraits::getNodeTypes() const
{
	return makeTypeInfoSet< Parameter >();
}

bool ParameterNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

bool ParameterNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType) const
{
	return isPinTypeScalar(pinType);
}

PinType ParameterNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes) const
{
	const Parameter* parameterNode = mandatory_non_null_type_cast< const Parameter* >(node);
	if (parameterNode->getDeclaration().getLength() > 1)
		return PinType::Array;
	else
	{
		switch (parameterNode->getDeclaration().getParameterType())
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
			return PinType::StructBuffer;
		default:
			return PinType::Void;
		}
	}
}

PinType ParameterNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes) const
{
	const Parameter* parameterNode = mandatory_non_null_type_cast< const Parameter* >(node);
	if (inputPin->getName() == L"Initial")
	{
		switch (parameterNode->getDeclaration().getParameterType())
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
			return PinType::StructBuffer;
		default:
			return PinType::Void;
		}
	}
	else
		return PinType::Void;
}

int32_t ParameterNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin) const
{
	return getInputPinIndex(node, inputPin);
}

bool ParameterNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant) const
{
	return false;
}

bool ParameterNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin) const
{
	return false;
}

PinOrder ParameterNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders) const
{
	return PinOrder::Constant;
}

}
