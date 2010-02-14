#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/SwizzleNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SwizzleNodeTraits", 0, SwizzleNodeTraits, INodeTraits)

TypeInfoSet SwizzleNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Swizzle >());
	return typeSet;
}

PinType SwizzleNodeTraits::getOutputPinType(
	const Node* node,
	const PinType* inputPinTypes,
	const OutputPin* outputPin
) const
{
	const std::wstring& pattern = checked_type_cast< const Swizzle* >(node)->get();
	switch (pattern.length())
	{
	case 1:
		return PntScalar1;
	case 2:
		return PntScalar2;
	case 3:
		return PntScalar3;
	case 4:
		return PntScalar4;
	default:
		return PntVoid;
	}
}

PinType SwizzleNodeTraits::getAcceptableInputPinType(
	const Node* node,
	const InputPin* inputPin
) const
{
	return PntScalar4;
}

	}
}
