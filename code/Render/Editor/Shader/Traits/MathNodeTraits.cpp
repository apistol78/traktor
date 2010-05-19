#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/MathNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MathNodeTraits", 0, MathNodeTraits, INodeTraits)

TypeInfoSet MathNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Abs >());
	typeSet.insert(&type_of< Add >());
	typeSet.insert(&type_of< ArcusCos >());
	typeSet.insert(&type_of< ArcusTan >());
	typeSet.insert(&type_of< Clamp >());
	typeSet.insert(&type_of< Cos >());
	typeSet.insert(&type_of< Cross >());
	typeSet.insert(&type_of< Derivative >());
	typeSet.insert(&type_of< Div >());
	typeSet.insert(&type_of< Exp >());
	typeSet.insert(&type_of< Fraction >());
	typeSet.insert(&type_of< Interpolator >());
	typeSet.insert(&type_of< Log >());
	typeSet.insert(&type_of< Max >());
	typeSet.insert(&type_of< Min >());
	typeSet.insert(&type_of< Mul >());
	typeSet.insert(&type_of< MulAdd >());
	typeSet.insert(&type_of< Neg >());
	typeSet.insert(&type_of< Normalize >());
	typeSet.insert(&type_of< Pow >());
	typeSet.insert(&type_of< Reflect >());
	typeSet.insert(&type_of< Sign >());
	typeSet.insert(&type_of< Sin >());
	typeSet.insert(&type_of< Sqrt >());
	typeSet.insert(&type_of< Sub >());
	typeSet.insert(&type_of< Tan >());
	return typeSet;
}

PinType MathNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	PinType outputPinType = PntVoid;

	uint32_t inputPinCount = node->getInputPinCount();
	for (uint32_t i = 0; i < inputPinCount; ++i)
		outputPinType = std::max< PinType >(
			outputPinType,
			inputPinTypes[i]
		);

	return outputPinType;
}

PinType MathNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	return outputPinTypes[0];
}

	}
}
