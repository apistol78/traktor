#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/TransposeNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TransposeNodeTraits", 0, TransposeNodeTraits, INodeTraits)

TypeInfoSet TransposeNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Transpose >());
	return typeSet;
}

PinType TransposeNodeTraits::getOutputPinType(
	const Node* node,
	const PinType* inputPinTypes,
	const OutputPin* outputPin
) const
{
	return inputPinTypes[0];
}

PinType TransposeNodeTraits::getAcceptableInputPinType(
	const Node* node,
	const InputPin* inputPin
) const
{
	return PntMatrix;
}

	}
}
