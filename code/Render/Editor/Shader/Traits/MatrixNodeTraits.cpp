#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/MatrixNodeTraits.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MatrixNodeTraits", 0, MatrixNodeTraits, INodeTraits)

TypeInfoSet MatrixNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MatrixIn >());
	typeSet.insert(&type_of< MatrixOut >());
	return typeSet;
}

bool MatrixNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

PinType MatrixNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (is_a< MatrixIn >(node))
		return PntMatrix;
	else if (is_a< MatrixOut >(node))
		return PntScalar4;
	else
		return PntVoid;
}

PinType MatrixNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (is_a< MatrixIn >(node))
		return PntScalar4;
	else if (is_a< MatrixOut >(node))
		return PntMatrix;
	else
		return PntVoid;
}

int32_t MatrixNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool MatrixNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool MatrixNodeTraits::evaluatePartial(
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

PinOrderType MatrixNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	if (is_a< MatrixIn >(node))
		return pinOrderConstantOrNonLinear(inputPinOrders, node->getInputPinCount());
	else if (is_a< MatrixOut >(node))
		return pinOrderMax(inputPinOrders, node->getInputPinCount());
	else
		return PotConstant;
}

	}
}
