#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/ConditionalNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ConditionalNodeTraits", 0, ConditionalNodeTraits, INodeTraits)

TypeInfoSet ConditionalNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Conditional >());
	return typeSet;
}

PinType ConditionalNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return std::max< PinType >(
		inputPinTypes[2],		// CaseTrue
		inputPinTypes[3]		// CaseFalse
	);
}

PinType ConditionalNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Input" || inputPin->getName() == L"Reference")
		return PntScalar1;
	else
		return outputPinTypes[0];
}

	}
}
