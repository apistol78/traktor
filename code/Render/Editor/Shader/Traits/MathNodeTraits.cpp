/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Traits/MathNodeTraits.h"

#include "Core/Math/Const.h"
#include "Render/Editor/Shader/Nodes.h"

#include <algorithm>

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MathNodeTraits", 0, MathNodeTraits, INodeTraits)

TypeInfoSet MathNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< Abs >();
	typeSet.insert< Add >();
	typeSet.insert< ArcusCos >();
	typeSet.insert< Clamp >();
	typeSet.insert< Cos >();
	typeSet.insert< Cross >();
	typeSet.insert< Derivative >();
	typeSet.insert< Div >();
	typeSet.insert< Exp >();
	typeSet.insert< Fraction >();
	typeSet.insert< Interpolator >();
	typeSet.insert< Log >();
	typeSet.insert< Max >();
	typeSet.insert< Min >();
	typeSet.insert< Mul >();
	typeSet.insert< MulAdd >();
	typeSet.insert< Neg >();
	typeSet.insert< Normalize >();
	typeSet.insert< Pow >();
	typeSet.insert< RecipSqrt >();
	typeSet.insert< Reflect >();
	typeSet.insert< Round >();
	typeSet.insert< Sign >();
	typeSet.insert< Sin >();
	typeSet.insert< Sqrt >();
	typeSet.insert< Sub >();
	typeSet.insert< Tan >();
	typeSet.insert< Truncate >();
	return typeSet;
}

bool MathNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

bool MathNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType) const
{
	return isPinTypeScalar(pinType);
}

PinType MathNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes) const
{
	PinType outputPinType = PinType::Void;
	if (is_a< ArcusCos >(node) || is_a< Cos >(node) || is_a< Sin >(node) || is_a< Tan >(node))
		outputPinType = PinType::Scalar1;
	else
	{
		// Conservativly use max pin type, even if any input pin is "void" since
		// there exists loops in the graph from Iterator nodes.
		uint32_t inputPinCount = node->getInputPinCount();
		for (uint32_t i = 0; i < inputPinCount; ++i)
			outputPinType = std::max< PinType >(
				outputPinType,
				inputPinTypes[i]);
	}
	return outputPinType;
}

PinType MathNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes) const
{
	if (is_a< ArcusCos >(node) || is_a< Cos >(node) || is_a< Sin >(node) || is_a< Tan >(node))
		return PinType::Scalar1;
	else
		return outputPinTypes[0];
}

int32_t MathNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin) const
{
	if (
		is_a< Add >(node) ||
		is_a< Mul >(node) ||
		is_a< Max >(node) ||
		is_a< Min >(node))
		return 0;
	else if (is_a< MulAdd >(node))
	{
		int32_t index = getInputPinIndex(node, inputPin);
		return (index == 0 || index == 1) ? 0 : 1;
	}
	else
		return getInputPinIndex(node, inputPin);
}

bool MathNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant) const
{
	if (is_a< Abs >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, std::abs(inputConstants[0].getValue(i)));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Add >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i) && inputConstants[1].isConst(i))
				outputConstant.setValue(i, inputConstants[0].getValue(i) + inputConstants[1].getValue(i));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< ArcusCos >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, std::acos(inputConstants[0].getValue(i)));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (const Clamp* clmp = dynamic_type_cast< const Clamp* >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, clamp(inputConstants[0].getValue(i), clmp->getMin(), clmp->getMax()));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Cos >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, std::cos(inputConstants[0].getValue(i)));
			else
				outputConstant.setVariant(i);
		return true;
	}
	// else if (is_a< Cross >(node))
	else if (is_a< Derivative >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, 0.0f);
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Div >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isZero(i) || inputConstants[1].isZero(i))
				outputConstant.setValue(i, 0.0f);
			else if (inputConstants[0].isConst(i) && inputConstants[1].isConst(i))
				outputConstant.setValue(i, inputConstants[0].getValue(i) / inputConstants[1].getValue(i));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Exp >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, std::exp(inputConstants[0].getValue(i)));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Fraction >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
			{
				float v = inputConstants[0].getValue(i);
				outputConstant.setValue(i, v - std::floor(v));
			}
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Interpolator >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, inputConstants[0].getValue(i));
			else
				outputConstant.setVariant(i);
		return true;
	}
	// else if (is_a< Log >(node))
	else if (is_a< Max >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i) && inputConstants[1].isConst(i))
				outputConstant.setValue(i, std::max(inputConstants[0].getValue(i), inputConstants[1].getValue(i)));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Min >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i) && inputConstants[1].isConst(i))
				outputConstant.setValue(i, std::min(inputConstants[0].getValue(i), inputConstants[1].getValue(i)));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Mul >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isZero(i) || inputConstants[1].isZero(i))
				outputConstant.setValue(i, 0.0f);
			else if (inputConstants[0].isConst(i) && inputConstants[1].isConst(i))
				outputConstant.setValue(i, inputConstants[0].getValue(i) * inputConstants[1].getValue(i));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< MulAdd >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
		{
			if (inputConstants[0].isZero(i) || inputConstants[1].isZero(i))
				if (inputConstants[2].isConst(i))
					outputConstant.setValue(i, inputConstants[2].getValue(i));
				else
					outputConstant.setVariant(i);
			else if (inputConstants[0].isConst(i) && inputConstants[1].isConst(i) && inputConstants[2].isConst(i))
				outputConstant.setValue(i, inputConstants[0].getValue(i) * inputConstants[1].getValue(i) + inputConstants[2].getValue(i));
			else
				outputConstant.setVariant(i);
		}
		return true;
	}
	else if (is_a< Neg >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, -inputConstants[0].getValue(i));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Normalize >(node))
	{
		if (inputConstants[0].isAllConst())
		{
			float ln = 0.0f;

			for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
				ln += inputConstants[0].getValue(i) * inputConstants[0].getValue(i);

			ln = std::sqrt(ln);

			if (std::abs(ln) >= FUZZY_EPSILON)
				for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
					outputConstant.setValue(i, inputConstants[0].getValue(i) / ln);
			else
				for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
					outputConstant.setValue(i, 0.0f);

			return true;
		}
		else
			return false;
	}
	else if (is_a< Pow >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isZero(i))
				outputConstant.setValue(i, 1.0f);
			else if (inputConstants[0].isConst(i) && inputConstants[1].isConst(i))
				outputConstant.setValue(i, std::pow(inputConstants[0].getValue(i), inputConstants[1].getValue(i)));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< RecipSqrt >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isZero(i))
				outputConstant.setValue(i, 0.0f);
			else if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, 1.0f / std::sqrt(inputConstants[0].getValue(i)));
			else
				outputConstant.setVariant(i);
		return true;
	}
	// else if (is_a< Reflect >(node))
	else if (is_a< Round >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, std::floor(inputConstants[0].getValue(i) + 0.5f));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Sign >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, inputConstants[0].getValue(i) >= 0.0f ? 1.0f : -1.0f);
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Sin >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, std::sin(inputConstants[0].getValue(i)));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Sqrt >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, std::sqrt(inputConstants[0].getValue(i)));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Sub >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i) && inputConstants[1].isConst(i))
				outputConstant.setValue(i, inputConstants[0].getValue(i) - inputConstants[1].getValue(i));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Tan >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, std::tan(inputConstants[0].getValue(i)));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else if (is_a< Truncate >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			if (inputConstants[0].isConst(i))
				outputConstant.setValue(i, std::floor(inputConstants[0].getValue(i)));
			else
				outputConstant.setVariant(i);
		return true;
	}
	else
		return false;
}

bool MathNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin) const
{
	// Discard node if inputs are constant.
	if (is_a< Add >(node))
	{
		if (inputConstants[0].isAllZero())
		{
			foldOutputPin = inputOutputPins[1];
			return true;
		}
		else if (inputConstants[1].isAllZero())
		{
			foldOutputPin = inputOutputPins[0];
			return true;
		}
	}
	else if (is_a< Div >(node))
	{
		if (inputConstants[1].isAllOne())
		{
			foldOutputPin = inputOutputPins[0];
			return true;
		}
	}
	else if (is_a< Mul >(node))
	{
		if (inputConstants[0].isAllOne())
		{
			foldOutputPin = inputOutputPins[1];
			return true;
		}
		else if (inputConstants[1].isAllOne())
		{
			foldOutputPin = inputOutputPins[0];
			return true;
		}
	}
	else if (is_a< MulAdd >(node))
	{
		if (inputConstants[0].isAllZero() || inputConstants[1].isAllZero())
		{
			foldOutputPin = inputOutputPins[2];
			return true;
		}
	}
	else if (is_a< Pow >(node))
	{
		if (inputConstants[0].isAllOne())
		{
			foldOutputPin = inputOutputPins[1];
			return true;
		}
	}
	else if (is_a< Sub >(node))
	{
		if (inputConstants[1].isAllZero())
		{
			foldOutputPin = inputOutputPins[0];
			return true;
		}
	}

	// Discard redundant nodes.
	if (is_a< Abs >(node))
	{
		if (inputOutputPins[0] != nullptr && is_a< Abs >(inputOutputPins[0]->getNode()))
		{
			foldOutputPin = inputOutputPins[0];
			return true;
		}
	}
	else if (is_a< Fraction >(node))
	{
		if (inputOutputPins[0] != nullptr && is_a< Fraction >(inputOutputPins[0]->getNode()))
		{
			foldOutputPin = inputOutputPins[0];
			return true;
		}
	}
	else if (is_a< Interpolator >(node))
	{
		if (inputOutputPins[0] != nullptr && is_a< Interpolator >(inputOutputPins[0]->getNode()))
		{
			foldOutputPin = inputOutputPins[0];
			return true;
		}
	}
	else if (is_a< Normalize >(node))
	{
		if (inputOutputPins[0] != nullptr && is_a< Normalize >(inputOutputPins[0]->getNode()))
		{
			foldOutputPin = inputOutputPins[0];
			return true;
		}
	}
	else if (is_a< Truncate >(node))
	{
		if (inputOutputPins[0] != nullptr && is_a< Truncate >(inputOutputPins[0]->getNode()))
		{
			foldOutputPin = inputOutputPins[0];
			return true;
		}
	}

	return false;
}

PinOrder MathNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders) const
{
	if (
		is_a< Abs >(node) ||
		is_a< ArcusCos >(node) ||
		is_a< Clamp >(node) ||
		is_a< Cos >(node) ||
		is_a< Cross >(node) ||
		is_a< Derivative >(node) ||
		is_a< Exp >(node) ||
		is_a< Fraction >(node) ||
		is_a< Log >(node) ||
		is_a< Max >(node) ||
		is_a< Min >(node) ||
		is_a< Normalize >(node) ||
		is_a< Pow >(node) ||
		is_a< RecipSqrt >(node) ||
		is_a< Reflect >(node) ||
		is_a< Round >(node) ||
		is_a< Sign >(node) ||
		is_a< Sin >(node) ||
		is_a< Sqrt >(node) ||
		is_a< Tan >(node) ||
		is_a< Truncate >(node))
		return pinOrderConstantOrNonLinear(inputPinOrders, node->getInputPinCount());
	else if (is_a< Add >(node))
		return pinOrderMax(inputPinOrders[0], inputPinOrders[1]);
	else if (is_a< Div >(node))
		return pinOrderAdd(inputPinOrders[0], inputPinOrders[1]);
	else if (is_a< Interpolator >(node))
		return inputPinOrders[0];
	else if (is_a< Mul >(node))
		return pinOrderAdd(inputPinOrders[0], inputPinOrders[1]);
	else if (is_a< MulAdd >(node))
		return pinOrderMax(pinOrderAdd(inputPinOrders[0], inputPinOrders[1]), inputPinOrders[2]);
	else if (is_a< Neg >(node))
		return inputPinOrders[0];
	else if (is_a< Sub >(node))
		return pinOrderMax(inputPinOrders[0], inputPinOrders[1]);
	else
		return PinOrder::Constant;
}

}
