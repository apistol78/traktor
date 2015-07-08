#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
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
	typeSet.insert(&type_of< Sampler >());
	return typeSet;
}

bool SamplerNodeTraits::isRoot(const Node* node) const
{
	return false;
}

PinType SamplerNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	const Sampler* samplerNode = checked_type_cast< const Sampler* >(node);
	return (samplerNode->getSamplerState().compare == CfNone) ? PntScalar4 : PntScalar1;
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
		return inputPinTypes[0]; //PntTexture;
	else if (inputPin->getName() == L"Mip")
		return PntScalar1;
	else
	{
		switch (inputPinTypes[0])
		{
		case PntTexture2D:
			return (samplerNode->getSamplerState().compare == CfNone) ? PntScalar2 : PntScalar3;
			break;

		case PntTexture3D:
		case PntTextureCube:
			return (samplerNode->getSamplerState().compare == CfNone) ? PntScalar3 : PntScalar4;
			break;

		default:
			return PntVoid;
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
