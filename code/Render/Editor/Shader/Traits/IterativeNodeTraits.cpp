#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/IterativeNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.IterativeNodeTraits", 0, IterativeNodeTraits, INodeTraits)

TypeInfoSet IterativeNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Iterate >());
	typeSet.insert(&type_of< Sum >());
	return typeSet;
}

PinType IterativeNodeTraits::getOutputPinType(
	const Node* node,
	const PinType* inputPinTypes,
	const OutputPin* outputPin
) const
{
	if (outputPin->getName() == L"N")
		return PntScalar1;
	else
		return inputPinTypes[0];
}

PinType IterativeNodeTraits::getAcceptableInputPinType(
	const Node* node,
	const InputPin* inputPin
) const
{
	return PntScalar4;
}

	}
}
