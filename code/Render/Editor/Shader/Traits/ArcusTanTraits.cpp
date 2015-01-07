#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/ArcusTanTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ArcusTanTraits", 0, ArcusTanTraits, INodeTraits)

TypeInfoSet ArcusTanTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ArcusTan >());
	return typeSet;
}

PinType ArcusTanTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return PntScalar1;
}

PinType ArcusTanTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	return PntScalar2;
}

int32_t ArcusTanTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return 0;
}

bool ArcusTanTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (inputConstants[0].isConstX() && inputConstants[0].isConstY())
	{
		outputConstant = Constant(
			std::atan2(inputConstants[0].x(), inputConstants[0].y())
		);
		return true;
	}
	else
		return false;
}

bool ArcusTanTraits::evaluatePartial(
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

PinOrderType ArcusTanTraits::evaluateOrder(
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
