#include <algorithm>
#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/LerpNodeTraits.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.LerpNodeTraits", 0, LerpNodeTraits, INodeTraits)

TypeInfoSet LerpNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Lerp >());
	return typeSet;
}

bool LerpNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

PinType LerpNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return std::max< PinType >(
		inputPinTypes[0],	// Input1
		inputPinTypes[1]	// Input2
	);
}

PinType LerpNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Blend")
		return PntScalar1;
	else
		return outputPinTypes[0];
}

int32_t LerpNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool LerpNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
	{
		if (
			inputConstants[0].isConst(i) &&
			inputConstants[1].isConst(i) &&
			inputConstants[2].isConst(i)
		)
		{
			float k = inputConstants[2].getValue(i);
			outputConstant.setValue(i, inputConstants[0].getValue(i) * (1.0f - k) + inputConstants[1].getValue(i) * k);
		}
	}
	return true;
}

bool LerpNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin
) const
{
	// If both inputs are constant and the values are equal then we can ignore
	// blend value even if it's not constant.
	if (inputConstants[0].isAllConst() && inputConstants[1].isAllConst())
	{
		if (inputConstants[0] == inputConstants[1])
		{
			foldOutputPin = inputOutputPins[0];
			return true;
		}
	}
	
	// If blend is constant zero.
	if (inputConstants[2].isAllZero())
	{
		foldOutputPin = inputOutputPins[0];
		return true;
	}
	
	// If blend is constant one.
	if (inputConstants[2].isAllOne())
	{
		foldOutputPin = inputOutputPins[1];
		return true;
	}

	return false;
}

PinOrderType LerpNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	return pinOrderMax(
		pinOrderAdd(inputPinOrders[0], inputPinOrders[2]),
		pinOrderAdd(inputPinOrders[1], inputPinOrders[2])
	);
}

	}
}
