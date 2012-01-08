#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/IterativeNodeTraits.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

int32_t getInputPinIndex(const Node* node, const InputPin* inputPin)
{
	int32_t inputPinCount = node->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		if (node->getInputPin(i) == inputPin)
			return i;
	}
	T_FATAL_ERROR;
	return -1;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.IterativeNodeTraits", 0, IterativeNodeTraits, INodeTraits)

TypeInfoSet IterativeNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Iterate >());
	typeSet.insert(&type_of< Sum >());
	return typeSet;
}

PinType IterativeNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (outputPin->getName() == L"N")
		return PntScalar1;
	else if (is_a< Iterate >(node))
	{
		return std::max< PinType >(
			inputPinTypes[0],			// Input
			inputPinTypes[1]			// Initial
		);
	}
	else
		return inputPinTypes[0];
}

PinType IterativeNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	return outputPinTypes[1];	// Output
}

int32_t IterativeNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool IterativeNodeTraits::evaluateFull(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool IterativeNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

	}
}
