#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/TransformNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TransformNodeTraits", 0, TransformNodeTraits, INodeTraits)

TypeInfoSet TransformNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Transform >());
	return typeSet;
}

PinType TransformNodeTraits::getOutputPinType(
	const Node* node,
	const PinType* inputPinTypes,
	const OutputPin* outputPin
) const
{
	return PntScalar4;
}

PinType TransformNodeTraits::getAcceptableInputPinType(
	const Node* node,
	const InputPin* inputPin
) const
{
	if (inputPin->getName() == L"Input")
		return PntScalar4;
	else
		return PntMatrix;
}

	}
}
