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

bool DotNodeTraits::evaluateFull(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	outputConstant[0] =
		inputConstants[0][0] * inputConstants[1][0] +
		inputConstants[0][1] * inputConstants[1][1] +
		inputConstants[0][2] * inputConstants[1][2] +
		inputConstants[0][3] * inputConstants[1][3];
	return true;
}

bool DotNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (inputConstants[0].isZero() || inputConstants[1].isZero())
	{
		outputConstant = Constant(0.0f);
		return true;
	}
	return false;
}

	}
}
