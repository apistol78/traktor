#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/ConditionalNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ConditionalNodeTraits", 0, ConditionalNodeTraits, INodeTraits)

TypeInfoSet ConditionalNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Conditional >());
	return typeSet;
}

PinType ConditionalNodeTraits::getOutputPinType(
	const Node* node,
	const PinType* inputPinTypes,
	const OutputPin* outputPin
) const
{
	return std::max< PinType >(
		inputPinTypes[2],		// CaseTrue
		inputPinTypes[3]		// CaseFalse
	);
}

PinType ConditionalNodeTraits::getAcceptableInputPinType(
	const Node* node,
	const InputPin* inputPin
) const
{
	if (inputPin->getName() == L"Input" || inputPin->getName() == L"Reference")
		return PntScalar1;
	else
		return PntScalar4;
}

	}
}
