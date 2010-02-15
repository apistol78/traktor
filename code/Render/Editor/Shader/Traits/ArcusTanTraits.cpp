#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/ArcusTanTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ArcusTanTraits", 0, ArcusTanTraits, INodeTraits)

TypeInfoSet ArcusTanTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ArcusTan >());
	return typeSet;
}

PinType ArcusTanTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return PntScalar1;
}

PinType ArcusTanTraits::getInputPinType(
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	return PntScalar2;
}

	}
}
