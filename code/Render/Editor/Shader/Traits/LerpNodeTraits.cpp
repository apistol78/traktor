#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/LerpNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.LerpNodeTraits", 0, LerpNodeTraits, INodeTraits)

TypeInfoSet LerpNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Lerp >());
	return typeSet;
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
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Blend")
		return PntScalar1;
	else
		return outputPinTypes[0];
}

bool LerpNodeTraits::evaluateFull(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	float k = inputConstants[2][0];
	for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
		outputConstant[i] = inputConstants[0][i] * (1.0f - k) + inputConstants[1][i] * k;
	return true;
}

bool LerpNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (inputConstants[2].isZero() && inputConstants[0].getWidth() > 0)
	{
		outputConstant = inputConstants[0];
		return true;
	}
	else if (inputConstants[2].isOne() && inputConstants[1].getWidth() > 0)
	{
		outputConstant = inputConstants[1];
		return true;
	}
	else
		return false;
}

	}
}
