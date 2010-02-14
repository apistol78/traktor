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
	const PinType* inputPinTypes,
	const OutputPin* outputPin
) const
{
	return std::max< PinType >(
		inputPinTypes[0],	// Input1
		inputPinTypes[1]	// Input2
	);
}

PinType LerpNodeTraits::getAcceptableInputPinType(
	const Node* node,
	const InputPin* inputPin
) const
{
	if (inputPin->getName() == L"Blend")
		return PntScalar1;
	else
		return PntScalar4;
}

	}
}
