#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/SwitchNodeTraits.h"

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
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Select")
		return PntScalar1;
	else
		return outputPinTypes[0];
}

int32_t SwitchNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool SwitchNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (inputConstants[0].getWidth() <= 0)
		return false;

	int32_t c = int32_t(inputConstants[0].x());
	if (c >= 0 && c < node->getInputPinCount() - 2)
		outputConstant = inputConstants[c + 2];
	else
		outputConstant = inputConstants[1];

	return outputConstant.getWidth() > 0;
}

bool SwitchNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin
) const
{
	if (inputConstants[0].getWidth() <= 0)
		return false;

	int32_t c = int32_t(inputConstants[0].x());
	if (c >= 0 && c < node->getInputPinCount() - 2)
		foldOutputPin = inputOutputPins[c + 2];
	else
		foldOutputPin = inputOutputPins[1];

	return true;
}

PinOrderType SwitchNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	return pinOrderConstantOrNonLinear(inputPinOrders, node->getInputPinCount());
}

	}
}
