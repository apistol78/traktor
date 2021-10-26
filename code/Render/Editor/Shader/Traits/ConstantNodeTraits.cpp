#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/ConstantNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ConstantNodeTraits", 0, ConstantNodeTraits, INodeTraits)

TypeInfoSet ConstantNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< Color >();
	typeSet.insert< Instance >();
	typeSet.insert< Scalar >();
	typeSet.insert< State >();
	typeSet.insert< Struct >();
	typeSet.insert< TargetSize >();
	typeSet.insert< Texture >();
	typeSet.insert< TextureSize >();
	typeSet.insert< Vector >();
	return typeSet;
}

bool ConstantNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

PinType ConstantNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (is_a< Color >(node) || is_a< Vector >(node))
		return PntScalar4;
	else if (is_a< Scalar >(node))
		return PntScalar1;
	else if (is_a< TargetSize >(node))
		return PntScalar2;
	else if (is_a< Instance >(node))
		return PntScalar1;
	else if (is_a< State >(node))
		return PntState;
	else if (is_a< Struct >(node))
		return PntStructBuffer;
	else if (is_a< Texture >(node))
	{
		const Texture* textureNode = checked_type_cast< const Texture*, false >(node);
		switch (textureNode->getParameterType())
		{
		case ParameterType::Texture2D:
			return PntTexture2D;
		case ParameterType::Texture3D:
			return PntTexture3D;
		case ParameterType::TextureCube:
			return PntTextureCube;
		default:
			return PntVoid;
		}
	}
	else if (is_a< TextureSize >(node))
	{
		if (inputPinTypes[0] == PntTexture2D)
			return PntScalar2;
		else
			return PntScalar3;
	}
	else
		return PntVoid;
}

PinType ConstantNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	return PntVoid;
}

int32_t ConstantNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return 0;
}

bool ConstantNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (const Color* color = dynamic_type_cast< const Color* >(node))
	{
		Color4f value = color->getColor();

		if (!color->getLinear())
			value = value.linear();

		outputConstant = Constant(
			value.getRed(),
			value.getGreen(),
			value.getBlue(),
			value.getAlpha()
		);
	}
	else if (const Vector* vectr = dynamic_type_cast< const Vector* >(node))
	{
		outputConstant = Constant(
			vectr->get().x(),
			vectr->get().y(),
			vectr->get().z(),
			vectr->get().w()
		);
	}
	else if (const Scalar* scalar = dynamic_type_cast< const Scalar* >(node))
	{
		outputConstant = Constant(scalar->get());
	}
	else
		return false;

	return true;
}

bool ConstantNodeTraits::evaluatePartial(
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

PinOrderType ConstantNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	return PotConstant;
}

	}
}
