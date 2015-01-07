#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/TransformNodeTraits.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TransformNodeTraits", 0, TransformNodeTraits, INodeTraits)

TypeInfoSet TransformNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Transform >());
	return typeSet;
}

PinType TransformNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return PntScalar4;
}

PinType TransformNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Input")
		return PntScalar4;
	else
		return PntMatrix;
}

int32_t TransformNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool TransformNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool TransformNodeTraits::evaluatePartial(
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

PinOrderType TransformNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	return pinOrderMax(inputPinOrders, node->getInputPinCount());
}

	}
}
