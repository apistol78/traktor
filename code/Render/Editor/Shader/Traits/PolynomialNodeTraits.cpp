#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/PolynomialNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.PolynomialNodeTraits", 0, PolynomialNodeTraits, INodeTraits)

TypeInfoSet PolynomialNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Polynomial >());
	return typeSet;
}

PinType PolynomialNodeTraits::getOutputPinType(
	const Node* node,
	const PinType* inputPinTypes,
	const OutputPin* outputPin
) const
{
	return PntScalar1;
}

PinType PolynomialNodeTraits::getAcceptableInputPinType(
	const Node* node,
	const InputPin* inputPin
) const
{
	if (inputPin->getName() == L"X")
		return PntScalar1;
	else
		return PntScalar4;
}

	}
}
