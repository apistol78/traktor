#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/PixelNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.PixelNodeTraits", 0, PixelNodeTraits, INodeTraits)

TypeInfoSet PixelNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PixelOutput >());
	return typeSet;
}

PinType PixelNodeTraits::getOutputPinType(
	const Node* node,
	const PinType* inputPinTypes,
	const OutputPin* outputPin
) const
{
	return PntVoid;
}

PinType PixelNodeTraits::getAcceptableInputPinType(
	const Node* node,
	const InputPin* inputPin
) const
{
	return PntScalar4;
}

	}
}
