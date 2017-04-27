/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/ConditionalNodeTraits.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

int32_t getInputPinIndex(const Node* node, const InputPin* inputPin)
{
	int32_t inputPinCount = node->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		if (node->getInputPin(i) == inputPin)
			return i;
	}
	T_FATAL_ERROR;
	return -1;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ConditionalNodeTraits", 0, ConditionalNodeTraits, INodeTraits)

TypeInfoSet ConditionalNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Conditional >());
	typeSet.insert(&type_of< Discard >());
	typeSet.insert(&type_of< Step >());
	return typeSet;
}

bool ConditionalNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

PinType ConditionalNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (is_a< Conditional >(node))
		return std::max< PinType >(
			inputPinTypes[2],		// CaseTrue
			inputPinTypes[3]		// CaseFalse
		);
	else if (is_a< Discard >(node))
		return inputPinTypes[2];	// Pass
	else if (is_a< Step >(node))
		return std::max< PinType >(
			inputPinTypes[0],		// X
			inputPinTypes[1]		// Y
		);
	else
		return PntVoid;
}

PinType ConditionalNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (is_a< Conditional >(node) || is_a< Discard >(node))
	{
		if (inputPin->getName() == L"Input" || inputPin->getName() == L"Reference")
			return PntScalar1;
	}
	return outputPinTypes[0];
}

int32_t ConditionalNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool ConditionalNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (const Conditional* conditional = dynamic_type_cast< const Conditional* >(node))
	{
		if (inputConstants[0].isConst(0) && inputConstants[1].isConst(0))
		{
			bool result = false;
			switch (conditional->getOperator())
			{
			case Conditional::CoLess:
				result = inputConstants[0].x() < inputConstants[1].x();
				break;

			case Conditional::CoLessEqual:
				result = inputConstants[0].x() <= inputConstants[1].x();
				break;

			case Conditional::CoEqual:
				result = inputConstants[0].x() == inputConstants[1].x();
				break;

			case Conditional::CoNotEqual:
				result = inputConstants[0].x() != inputConstants[1].x();
				break;

			case Conditional::CoGreater:
				result = inputConstants[0].x() > inputConstants[1].x();
				break;

			case Conditional::CoGreaterEqual:
				result = inputConstants[0].x() >= inputConstants[1].x();
				break;

			default:
				return false;
			}

			for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			{
				if (result)
				{
					if (inputConstants[2].isConst(i))
						outputConstant.setValue(i, inputConstants[2].getValue(i));
					else
						outputConstant.setVariant(i);
				}
				else
				{
					if (inputConstants[3].isConst(i))
						outputConstant.setValue(i, inputConstants[3].getValue(i));
					else
						outputConstant.setVariant(i);
				}
			}

			return true;
		}
	}
	else if (const Discard* discard = dynamic_type_cast< const Discard* >(node))
	{
		if (inputConstants[0].isConst(0) && inputConstants[1].isConst(0))
		{
			bool result = false;
			switch (discard->getOperator())
			{
			case Discard::CoLess:
				result = inputConstants[0].x() < inputConstants[1].x();
				break;

			case Discard::CoLessEqual:
				result = inputConstants[0].x() <= inputConstants[1].x();
				break;

			case Discard::CoEqual:
				result = inputConstants[0].x() == inputConstants[1].x();
				break;

			case Discard::CoNotEqual:
				result = inputConstants[0].x() != inputConstants[1].x();
				break;

			case Discard::CoGreater:
				result = inputConstants[0].x() > inputConstants[1].x();
				break;

			case Discard::CoGreaterEqual:
				result = inputConstants[0].x() >= inputConstants[1].x();
				break;

			default:
				return false;
			}

			if (result)
			{
				for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
				{
					if (inputConstants[2].isConst(i))
						outputConstant.setValue(i, inputConstants[2].getValue(i));
					else
						outputConstant.setVariant(i);
				}
			}
			else
			{
				outputConstant = Constant();
			}

			return true;
		}
	}
	else if (const Step* step = dynamic_type_cast< const Step* >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
		{
			if (inputConstants[0].isConst(i) && inputConstants[1].isConst(i))
				outputConstant.setValue(i, (inputConstants[0].getValue(i) >= inputConstants[1].getValue(i)) ? 1.0f : 0.0f);
			else
				outputConstant.setVariant(i);
		}
		return true;
	}
	return false;
}

bool ConditionalNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin
) const
{
	if (const Conditional* conditional = dynamic_type_cast< const Conditional* >(node))
	{
		if (inputConstants[0].isConst(0) && inputConstants[1].isConst(0))
		{
			bool result = false;
			switch (conditional->getOperator())
			{
			case Conditional::CoLess:
				result = inputConstants[0].x() < inputConstants[1].x();
				break;

			case Conditional::CoLessEqual:
				result = inputConstants[0].x() <= inputConstants[1].x();
				break;

			case Conditional::CoEqual:
				result = inputConstants[0].x() == inputConstants[1].x();
				break;

			case Conditional::CoNotEqual:
				result = inputConstants[0].x() != inputConstants[1].x();
				break;

			case Conditional::CoGreater:
				result = inputConstants[0].x() > inputConstants[1].x();
				break;

			case Conditional::CoGreaterEqual:
				result = inputConstants[0].x() >= inputConstants[1].x();
				break;

			default:
				return false;
			}

			if (result)
				foldOutputPin = inputOutputPins[2];
			else
				foldOutputPin = inputOutputPins[3];

			return true;
		}
	}
	else if (const Discard* discard = dynamic_type_cast< const Discard* >(node))
	{
		if (inputConstants[0].isConst(0) && inputConstants[1].isConst(0))
		{
			bool result = false;
			switch (discard->getOperator())
			{
			case Discard::CoLess:
				result = inputConstants[0].x() < inputConstants[1].x();
				break;

			case Discard::CoLessEqual:
				result = inputConstants[0].x() <= inputConstants[1].x();
				break;

			case Discard::CoEqual:
				result = inputConstants[0].x() == inputConstants[1].x();
				break;

			case Discard::CoNotEqual:
				result = inputConstants[0].x() != inputConstants[1].x();
				break;

			case Discard::CoGreater:
				result = inputConstants[0].x() > inputConstants[1].x();
				break;

			case Discard::CoGreaterEqual:
				result = inputConstants[0].x() >= inputConstants[1].x();
				break;

			default:
				return false;
			}

			if (result)
				foldOutputPin = inputOutputPins[2];
			else
				foldOutputPin = 0;

			return true;
		}
	}
	return false;
}

PinOrderType ConditionalNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	return pinOrderConstantOrNonLinear(inputPinOrders, node->getInputPinCount());
}

	}
}
