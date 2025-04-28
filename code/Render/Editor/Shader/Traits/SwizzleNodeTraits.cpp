/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Traits/SwizzleNodeTraits.h"

#include "Render/Editor/Shader/Nodes.h"

#include <algorithm>
#include <cctype>

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SwizzleNodeTraits", 0, SwizzleNodeTraits, INodeTraits)

TypeInfoSet SwizzleNodeTraits::getNodeTypes() const
{
	return makeTypeInfoSet< Swizzle >();
}

bool SwizzleNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

bool SwizzleNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType) const
{
	return isPinTypeScalar(pinType);
}

PinType SwizzleNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes) const
{
	const std::wstring& pattern = checked_type_cast< const Swizzle* >(node)->get();
	switch (pattern.length())
	{
	case 1:
		return PinType::Scalar1;
	case 2:
		return PinType::Scalar2;
	case 3:
		return PinType::Scalar3;
	case 4:
		return PinType::Scalar4;
	default:
		return PinType::Void;
	}
}

PinType SwizzleNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes) const
{
	const std::wstring& pattern = checked_type_cast< const Swizzle* >(node)->get();

	PinType inputPinType = PinType::Void;
	for (size_t i = 0; i < pattern.length(); ++i)
	{
		switch (std::tolower(pattern[i]))
		{
		case L'x':
			inputPinType = std::max< PinType >(inputPinType, PinType::Scalar1);
			break;
		case L'y':
			inputPinType = std::max< PinType >(inputPinType, PinType::Scalar2);
			break;
		case L'z':
			inputPinType = std::max< PinType >(inputPinType, PinType::Scalar3);
			break;
		case L'w':
			inputPinType = std::max< PinType >(inputPinType, PinType::Scalar4);
			break;
		}
	}

	return inputPinType;
}

int32_t SwizzleNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin) const
{
	return 0;
}

bool SwizzleNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant) const
{
	const std::wstring& pattern = checked_type_cast< const Swizzle* >(node)->get();
	for (int32_t i = 0; i < int32_t(pattern.length()); ++i)
	{
		switch (std::tolower(pattern[i]))
		{
		case L'x':
			if (inputConstants[0].isConstX())
				outputConstant.setValue(i, inputConstants[0].x());
			else
				outputConstant.setVariant(i);
			break;
		case L'y':
			if (inputConstants[0].isConstY())
				outputConstant.setValue(i, inputConstants[0].y());
			else
				outputConstant.setVariant(i);
			break;
		case L'z':
			if (inputConstants[0].isConstZ())
				outputConstant.setValue(i, inputConstants[0].z());
			else
				outputConstant.setVariant(i);
			break;
		case L'w':
			if (inputConstants[0].isConstW())
				outputConstant.setValue(i, inputConstants[0].w());
			else
				outputConstant.setVariant(i);
			break;
		case L'0':
			outputConstant.setValue(i, 0.0f);
			break;
		case L'1':
			outputConstant.setValue(i, 1.0f);
			break;
		}
	}
	return true;
}

bool SwizzleNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin) const
{
	return false;
}

PinOrder SwizzleNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders) const
{
	return inputPinOrders[0];
}

}
