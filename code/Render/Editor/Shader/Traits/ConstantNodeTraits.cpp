#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/ConstantNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ConstantNodeTraits", 0, ConstantNodeTraits, INodeTraits)

TypeInfoSet ConstantNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Color >());
	typeSet.insert(&type_of< FragmentPosition >());
	typeSet.insert(&type_of< Scalar >());
	typeSet.insert(&type_of< Texture >());
	typeSet.insert(&type_of< Vector >());
	return typeSet;
}

PinType ConstantNodeTraits::getOutputPinType(
	const Node* node,
	const PinType* inputPinTypes,
	const OutputPin* outputPin
) const
{
	if (is_a< Color >(node) || is_a< FragmentPosition >(node) || is_a< Vector >(node))
		return PntScalar4;
	else if (is_a< Scalar >(node))
		return PntScalar1;
	else if (is_a< Texture >(node))
		return PntTexture;
	else
		return PntVoid;
}

PinType ConstantNodeTraits::getAcceptableInputPinType(
	const Node* node,
	const InputPin* inputPin
) const
{
	return PntVoid;
}

	}
}
