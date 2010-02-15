#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/LengthNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.LengthNodeTraits", 0, LengthNodeTraits, INodeTraits)

TypeInfoSet LengthNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Length >());
	return typeSet;
}

PinType LengthNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return PntScalar1;
}

PinType LengthNodeTraits::getInputPinType(
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	return PntScalar4;
}

	}
}
