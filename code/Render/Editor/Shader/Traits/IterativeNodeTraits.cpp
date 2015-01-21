#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/IterativeNodeTraits.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.IterativeNodeTraits", 0, IterativeNodeTraits, INodeTraits)

TypeInfoSet IterativeNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Iterate >());
	typeSet.insert(&type_of< Iterate2d >());
	typeSet.insert(&type_of< Repeat >());
	typeSet.insert(&type_of< Sum >());
	return typeSet;
}

bool IterativeNodeTraits::isRoot(const Node* node) const
{
	return false;
}

PinType IterativeNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (
		outputPin->getName() == L"N" ||
		outputPin->getName() == L"X" ||
		outputPin->getName() == L"Y"
	)
		return PntScalar1;
	else if (
		is_a< Iterate >(node) ||
		is_a< Iterate2d >(node) ||
		is_a< Repeat >(node)
	)
	{
		return std::max< PinType >(
			inputPinTypes[0],			// Input
			inputPinTypes[1]			// Initial
		);
	}
	else
		return inputPinTypes[0];
}

PinType IterativeNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Condition")
		return PntScalar1;
	else
		return outputPinTypes[1];	// Output
}

int32_t IterativeNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool IterativeNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool IterativeNodeTraits::evaluatePartial(
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

PinOrderType IterativeNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	if (
		nodeOutputPin->getName() == L"N" ||
		nodeOutputPin->getName() == L"X" ||
		nodeOutputPin->getName() == L"Y"
	)
		return PotConstant;
	
	if (
		is_a< Iterate >(node) ||
		is_a< Iterate2d >(node) ||
		is_a< Repeat >(node)
	)
		return pinOrderMax(
			inputPinOrders[0],
			inputPinOrders[1]
		);
	else	// Sum
		return inputPinOrders[0];
}

	}
}
