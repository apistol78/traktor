#include <cctype>
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
	const OutputPin* outputPin,
	const PinType* inputPinTypes
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

PinType SwizzleNodeTraits::getInputPinType(
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	const std::wstring& pattern = checked_type_cast< const Swizzle* >(node)->get();
	
	PinType inputPinType = PntVoid;
	for (size_t i = 0; i < pattern.length(); ++i)
	{
		switch (std::tolower(pattern[i]))
		{
		case L'x':
			inputPinType = std::max< PinType >(inputPinType, PntScalar1);
			break;
		case L'y':
			inputPinType = std::max< PinType >(inputPinType, PntScalar2);
			break;
		case L'z':
			inputPinType = std::max< PinType >(inputPinType, PntScalar3);
			break;
		case L'w':
			inputPinType = std::max< PinType >(inputPinType, PntScalar4);
			break;
		}
	}

	return inputPinType;
}

	}
}
