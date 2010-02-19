#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/SwitchNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SwitchNodeTraits", 0, SwitchNodeTraits, INodeTraits)

TypeInfoSet SwitchNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Switch >());
	return typeSet;
}

PinType SwitchNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	PinType outputType = PntVoid;

	uint32_t inputPinCount = node->getInputPinCount();
	for (uint32_t i = 1; i < inputPinCount; ++i)
		outputType = std::max< PinType >(
			outputType,
			inputPinTypes[i]
		);

	return outputType;
}

PinType SwitchNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Select")
		return PntScalar1;
	else
		return outputPinTypes[0];
}

	}
}
