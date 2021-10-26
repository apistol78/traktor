#include <algorithm>
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/SwitchNodeTraits.h"

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

int32_t getOutputPinIndex(const Node* node, const OutputPin* outputPin)
{
	int32_t outputPinCount = node->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		if (node->getOutputPin(i) == outputPin)
			return i;
	}
	T_FATAL_ERROR;
	return -1;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SwitchNodeTraits", 0, SwitchNodeTraits, INodeTraits)

TypeInfoSet SwitchNodeTraits::getNodeTypes() const
{
	return makeTypeInfoSet< Switch >();
}

bool SwitchNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

PinType SwitchNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	PinType outputType = PinType::Void;

	uint32_t inputPinCount = node->getInputPinCount();
	for (uint32_t i = 1; i < inputPinCount; ++i)
		outputType = std::max< PinType >(
			outputType,
			inputPinTypes[i]
		);

	return outputType;
}

PinType SwitchNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	const Switch* switchNode = mandatory_non_null_type_cast< const Switch* >(node);
	const int32_t index = getInputPinIndex(switchNode, inputPin);
	if (index <= 0)
	{
		// Select
		return PinType::Scalar1;
	}
	else
	{
		int32_t output = (index - 1) % switchNode->getWidth();
		return outputPinTypes[output];
	}
}

int32_t SwitchNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool SwitchNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	const Switch* switchNode = mandatory_non_null_type_cast< const Switch* >(node);
	const int32_t width = switchNode->getWidth();
	const auto& cases = switchNode->getCases();

	// Get which output pin is evaluated.
	const int32_t output = getOutputPinIndex(switchNode, nodeOutputPin);

	// Select pin must be constant if we're to evaluate switch.
	if (!inputConstants[0].isConstX())
		return false;

	int32_t c = int32_t(inputConstants[0].x());

	// Find case branch.
	bool foundBranch = false;
	for (int32_t i = 0; i < (int32_t)cases.size(); ++i)
	{
		if (cases[i] == c)
		{
			outputConstant = inputConstants[1 + (1 + i) * width + output];
			foundBranch = true;
			break;
		}
	}

	// Use default branch if no case found.
	if (!foundBranch)
		outputConstant = inputConstants[1 + output];

	return outputConstant.getWidth() > 0;
}

bool SwitchNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin
) const
{
	const Switch* switchNode = mandatory_non_null_type_cast< const Switch* >(node);
	const int32_t width = switchNode->getWidth();
	const auto& cases = switchNode->getCases();

	// Get which output pin is evaluated.
	const int32_t output = getOutputPinIndex(switchNode, nodeOutputPin);

	// Select pin must be constant if we're to evaluate switch.
	if (!inputConstants[0].isConstX())
		return false;

	int32_t c = int32_t(inputConstants[0].x());

	// Find case branch.
	bool foundBranch = false;
	for (int32_t i = 0; i < (int32_t)cases.size(); ++i)
	{
		if (cases[i] == c)
		{
			foldOutputPin = inputOutputPins[1 + (1 + i) * width + output];
			foundBranch = true;
			break;
		}
	}

	// Use default branch if no case found.
	if (!foundBranch)
		foldOutputPin = inputOutputPins[1 + output];

	return true;
}

PinOrderType SwitchNodeTraits::evaluateOrder(
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
