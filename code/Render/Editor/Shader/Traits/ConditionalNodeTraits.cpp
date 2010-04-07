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
	typeSet.insert(&type_of< Discard >());
	return typeSet;
}

PinType ConditionalNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (is_a< Conditional >(node))
		return std::max< PinType >(
			inputPinTypes[2],		// CaseTrue
			inputPinTypes[3]		// CaseFalse
		);
	else if (is_a< Discard >(node))
		return inputPinTypes[2];	// Pass
	else
		return PntVoid;
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
