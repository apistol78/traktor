/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/UniformNodeTraits.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.UniformNodeTraits", 0, UniformNodeTraits, INodeTraits)

TypeInfoSet UniformNodeTraits::getNodeTypes() const
{
	return makeTypeInfoSet<
		IndexedUniform,
		Uniform
	>();
}

bool UniformNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

bool UniformNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType
) const
{
	return isPinTypeScalar(pinType);
}

PinType UniformNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	ParameterType parameterType;

	if (const IndexedUniform* indexedUniformNode = dynamic_type_cast< const IndexedUniform* >(node))
		parameterType = indexedUniformNode->getParameterType();
	else if (const Uniform* uniformNode = dynamic_type_cast< const Uniform* >(node))
		parameterType = uniformNode->getParameterType();
	else
		return PinType::Void;

	switch (parameterType)
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

PinType UniformNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (is_a< IndexedUniform >(node))
		return PinType::Scalar1;
	else if (const Uniform* uniformNode = dynamic_type_cast< const Uniform* >(node))
	{
		if (inputPin->getName() == L"Initial")
		{
			switch (uniformNode->getParameterType())
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
	else
		return PinType::Void;
}

int32_t UniformNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool UniformNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool UniformNodeTraits::evaluatePartial(
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

PinOrder UniformNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders
) const
{
	if (const IndexedUniform* indexedUniform = dynamic_type_cast< const IndexedUniform* >(node))
	{
		if (inputPinOrders[0] == PinOrder::Constant)
			return PinOrder::Constant;
		else
			return PinOrder::NonLinear;
	}
	else if (const Uniform* uniform = dynamic_type_cast< const Uniform* >(node))
	{
		return PinOrder::Constant;
	}
	else
		return PinOrder::Constant;
}

	}
}
