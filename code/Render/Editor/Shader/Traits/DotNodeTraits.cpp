#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/DotNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.DotNodeTraits", 0, DotNodeTraits, INodeTraits)

TypeInfoSet DotNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Dot >());
	return typeSet;
}

bool DotNodeTraits::isRoot(const Node* node) const
{
	return false;
}

PinType DotNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return PntScalar1;
}

PinType DotNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	return PntScalar4;
}

int32_t DotNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return 0;
}

bool DotNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (inputConstants[0].isAllConst() && inputConstants[1].isAllConst())
	{
		outputConstant = Constant(
			inputConstants[0].x() * inputConstants[1].x() +
			inputConstants[0].y() * inputConstants[1].y() +
			inputConstants[0].z() * inputConstants[1].z() +
			inputConstants[0].w() * inputConstants[1].w()
		);
	}
	else if (inputConstants[0].isAllZero() || inputConstants[1].isAllZero())
	{
		outputConstant = Constant(0.0f);
		return true;
	}
	return false;
}

bool DotNodeTraits::evaluatePartial(
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

PinOrderType DotNodeTraits::evaluateOrder(
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
