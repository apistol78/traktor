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

bool MathNodeTraits::evaluate(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (is_a< Abs >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::abs(inputConstants[0][i]);
	}
	else if (is_a< Add >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = inputConstants[0][i] + inputConstants[1][i];
	}
	else if (is_a< ArcusCos >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::acos(inputConstants[0][i]);
	}
	else if (const Clamp* clmp = dynamic_type_cast< const Clamp* >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = clamp(inputConstants[0][i], clmp->getMin(), clmp->getMax());
	}
	else if (is_a< Cos >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::cos(inputConstants[0][i]);
	}
	//else if (is_a< Cross >(node))
	else if (is_a< Derivative >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = 0.0f;
	}
	else if (is_a< Div >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = inputConstants[0][i] / inputConstants[1][i];
	}
	//else if (is_a< Exp >(node))
	//else if (is_a< Fraction >(node))
	else if (is_a< Interpolator >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = inputConstants[0][i];
	}
	//else if (is_a< Log >(node))
	else if (is_a< Max >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::max(inputConstants[0][i], inputConstants[1][i]);
	}
	else if (is_a< Min >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::min(inputConstants[0][i], inputConstants[1][i]);
	}
	else if (is_a< Mul >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = inputConstants[0][i] * inputConstants[1][i];
	}
	else if (is_a< MulAdd >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = inputConstants[0][i] * inputConstants[1][i] + inputConstants[2][i];
	}
	else if (is_a< Neg >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = -inputConstants[0][i];
	}
	else if (is_a< Normalize >(node))
	{
		float ln = std::sqrt(
			inputConstants[0][0] * inputConstants[0][0] +
			inputConstants[0][1] * inputConstants[0][1] +
			inputConstants[0][2] * inputConstants[0][2] +
			inputConstants[0][3] * inputConstants[0][3]
		);
		if (ln != 0.0f)
		{
			for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
				outputConstant[i] = inputConstants[0][i] / ln;
		}
		else
		{
			for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
				outputConstant[i] = 0.0f;
		}
	}
	else if (is_a< Pow >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::powf(inputConstants[0][i], inputConstants[1][i]);
	}
	//else if (is_a< Reflect >(node))
	else if (is_a< Sign >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = (inputConstants[0][i] >= 0.0f) ? 1.0f : -1.0f;
	}
	else if (is_a< Sin >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::sin(inputConstants[0][i]);
	}
	else if (is_a< Sqrt >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::sqrtf(inputConstants[0][i]);
	}
	else if (is_a< Sub >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = inputConstants[0][i] - inputConstants[1][i];
	}
	else if (is_a< Tan >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
			outputConstant[i] = std::tan(inputConstants[0][i]);
	}
	else
		return false;

	return true;
}

	}
}
