/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/ConstantNodeTraits.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ConstantNodeTraits", 0, ConstantNodeTraits, INodeTraits)

TypeInfoSet ConstantNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< Color >();
	typeSet.insert< Instance >();
	typeSet.insert< PixelState >();
	typeSet.insert< Scalar >();
	typeSet.insert< Struct >();
	typeSet.insert< TargetSize >();
	typeSet.insert< Texture >();
	typeSet.insert< TextureSize >();
	typeSet.insert< TextureState >();
	typeSet.insert< Vector >();
	return typeSet;
}

bool ConstantNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

bool ConstantNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType
) const
{
	if (is_a< TextureSize >(node))
		return isPinTypeTexture(pinType);
	else
		return false;
}

PinType ConstantNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (is_a< Color >(node) || is_a< Vector >(node))
		return PinType::Scalar4;
	else if (is_a< Scalar >(node))
		return PinType::Scalar1;
	else if (is_a< TargetSize >(node))
		return PinType::Scalar2;
	else if (is_a< Instance >(node))
		return PinType::Scalar1;
	else if (is_a< PixelState >(node))
		return PinType::State;
	else if (is_a< Struct >(node))
		return PinType::StructBuffer;
	else if (is_a< Texture >(node))
	{
		const Texture* textureNode = checked_type_cast< const Texture*, false >(node);
		switch (textureNode->getParameterType())
		{
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
	else if (is_a< TextureSize >(node))
	{
		if (inputPinTypes[0] == PinType::Texture2D)
			return PinType::Scalar2;
		else
			return PinType::Scalar3;
	}
	else if (is_a< TextureState >(node))
		return PinType::State;
	else
		return PinType::Void;
}

PinType ConstantNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	return PinType::Void;
}

int32_t ConstantNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return 0;
}

bool ConstantNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (const Color* color = dynamic_type_cast< const Color* >(node))
	{
		Color4f value = color->getColor();

		if (!color->getLinear())
			value = value.linear();

		outputConstant = Constant(
			value.getRed(),
			value.getGreen(),
			value.getBlue(),
			value.getAlpha()
		);
	}
	else if (const Vector* vectr = dynamic_type_cast< const Vector* >(node))
	{
		outputConstant = Constant(
			vectr->get().x(),
			vectr->get().y(),
			vectr->get().z(),
			vectr->get().w()
		);
	}
	else if (const Scalar* scalar = dynamic_type_cast< const Scalar* >(node))
	{
		outputConstant = Constant(scalar->get());
	}
	else
		return false;

	return true;
}

bool ConstantNodeTraits::evaluatePartial(
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

PinOrder ConstantNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders
) const
{
	return PinOrder::Constant;
}

}
