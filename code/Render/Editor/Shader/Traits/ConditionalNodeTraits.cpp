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
	return typeSet;
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
	else
		return PntVoid;
}

PinType ConditionalNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Input" || inputPin->getName() == L"Reference")
		return PntScalar1;
	else
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

bool ConditionalNodeTraits::evaluateFull(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (const Conditional* conditional = dynamic_type_cast< const Conditional* >(node))
	{
		bool result = false;
		switch (conditional->getOperator())
		{
		case Conditional::CoLess:
			result = inputConstants[0][0] < inputConstants[1][0];
			break;

		case Conditional::CoLessEqual:
			result = inputConstants[0][0] <= inputConstants[1][0];
			break;

		case Conditional::CoEqual:
			result = inputConstants[0][0] == inputConstants[1][0];
			break;

		case Conditional::CoNotEqual:
			result = inputConstants[0][0] != inputConstants[1][0];
			break;

		case Conditional::CoGreater:
			result = inputConstants[0][0] > inputConstants[1][0];
			break;

		case Conditional::CoGreaterEqual:
			result = inputConstants[0][0] >= inputConstants[1][0];
			break;

		default:
			return false;
		}

		if (result)
			outputConstant = inputConstants[2];
		else
			outputConstant = inputConstants[3];

		return true;
	}

	return false;
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
		if (inputConstants[0].getWidth() <= 0 || inputConstants[1].getWidth() <= 0)
			return false;

		bool result = false;
		switch (conditional->getOperator())
		{
		case Conditional::CoLess:
			result = inputConstants[0][0] < inputConstants[1][0];
			break;

		case Conditional::CoLessEqual:
			result = inputConstants[0][0] <= inputConstants[1][0];
			break;

		case Conditional::CoEqual:
			result = inputConstants[0][0] == inputConstants[1][0];
			break;

		case Conditional::CoNotEqual:
			result = inputConstants[0][0] != inputConstants[1][0];
			break;

		case Conditional::CoGreater:
			result = inputConstants[0][0] > inputConstants[1][0];
			break;

		case Conditional::CoGreaterEqual:
			result = inputConstants[0][0] >= inputConstants[1][0];
			break;

		default:
			return false;
		}

		if (result)
		{
			if (inputConstants[2].getWidth() > 0)
			{
				outputConstant = inputConstants[2];
				return true;
			}
		}
		else
		{
			if (inputConstants[3].getWidth() > 0)
			{
				outputConstant = inputConstants[3];
				return true;
			}
		}
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
		if (inputConstants[0].getWidth() <= 0 || inputConstants[1].getWidth() <= 0)
			return false;

		bool result = false;
		switch (conditional->getOperator())
		{
		case Conditional::CoLess:
			result = inputConstants[0][0] < inputConstants[1][0];
			break;

		case Conditional::CoLessEqual:
			result = inputConstants[0][0] <= inputConstants[1][0];
			break;

		case Conditional::CoEqual:
			result = inputConstants[0][0] == inputConstants[1][0];
			break;

		case Conditional::CoNotEqual:
			result = inputConstants[0][0] != inputConstants[1][0];
			break;

		case Conditional::CoGreater:
			result = inputConstants[0][0] > inputConstants[1][0];
			break;

		case Conditional::CoGreaterEqual:
			result = inputConstants[0][0] >= inputConstants[1][0];
			break;

		default:
			return false;
		}

		if (result)
		{
			foldOutputPin = inputOutputPins[2];
			return true;
		}
		else
		{
			foldOutputPin = inputOutputPins[3];
			return true;
		}
	}
	return false;
}

	}
}
