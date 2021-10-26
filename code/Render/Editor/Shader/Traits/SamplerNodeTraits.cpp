#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Traits/SamplerNodeTraits.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SamplerNodeTraits", 0, SamplerNodeTraits, INodeTraits)

TypeInfoSet SamplerNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< Sampler >();
	return typeSet;
}

bool SamplerNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

PinType SamplerNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	const Sampler* samplerNode = checked_type_cast< const Sampler* >(node);
	return (samplerNode->getSamplerState().compare == CfNone) ? PinType::Scalar4 : PinType::Scalar1;
}

PinType SamplerNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	const Sampler* samplerNode = checked_type_cast< const Sampler* >(node);
	if (inputPin->getName() == L"Texture")
		return inputPinTypes[0];
	else if (inputPin->getName() == L"Mip")
		return PinType::Scalar1;
	else
	{
		switch (inputPinTypes[0])
		{
		case PinType::Texture2D:
			return (samplerNode->getSamplerState().compare == CfNone) ? PinType::Scalar2 : PinType::Scalar3;

		case PinType::Texture3D:
		case PinType::TextureCube:
			return (samplerNode->getSamplerState().compare == CfNone) ? PinType::Scalar3 : PinType::Scalar4;

		default:
			return PinType::Void;
		}
	}
}

int32_t SamplerNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool SamplerNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool SamplerNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const OutputPin** inputOutputPins,
	const Constant* inputConstants,
	const OutputPin*& foldOutputPin
) const
{
	return false;
}

PinOrderType SamplerNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	return PotNonLinear;
}

	}
}
