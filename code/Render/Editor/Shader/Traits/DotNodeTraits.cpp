#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/DotNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.DotNodeTraits", 0, DotNodeTraits, INodeTraits)

TypeInfoSet DotNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Dot >());
	return typeSet;
}

PinType DotNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return PntScalar1;
}

PinType DotNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	return PntScalar4;
}

	}
}
