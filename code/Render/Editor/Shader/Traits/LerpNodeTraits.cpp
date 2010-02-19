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
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return std::max< PinType >(
		inputPinTypes[0],	// Input1
		inputPinTypes[1]	// Input2
	);
}

PinType LerpNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Blend")
		return PntScalar1;
	else
		return outputPinTypes[0];
}

	}
}
