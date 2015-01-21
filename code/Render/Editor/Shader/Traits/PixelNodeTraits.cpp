#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Traits/PixelNodeTraits.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.PixelNodeTraits", 0, PixelNodeTraits, INodeTraits)

TypeInfoSet PixelNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PixelOutput >());
	return typeSet;
}

bool PixelNodeTraits::isRoot(const Node* node) const
{
	return true;
}

PinType PixelNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return PntVoid;
}

PinType PixelNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	const PixelOutput* pixelOutputNode = checked_type_cast< const PixelOutput* >(node);

	RenderState rs = pixelOutputNode->getRenderState();

	const OutputPin* statePin = shaderGraph->findSourcePin(pixelOutputNode->getInputPin(4));
	if (statePin)
	{
		const State* state = dynamic_type_cast< const State* >(statePin->getNode());
		if (!state)
			return PntScalar4;

		rs = state->getRenderState();
	}

	if (!rs.blendEnable && !rs.alphaTestEnable)
	{
		if (rs.colorWriteMask & CwAlpha)
			return PntScalar4;
		else if (rs.colorWriteMask & CwBlue)
			return PntScalar3;
		else if (rs.colorWriteMask & CwGreen)
			return PntScalar2;
		else if (rs.colorWriteMask & CwRed)
			return PntScalar1;
		else
			return PntVoid;
	}
	else
	{
		if (rs.blendEnable)
		{
			if (
				rs.blendSource == BfSourceAlpha ||
				rs.blendSource == BfOneMinusSourceAlpha ||
				rs.blendDestination == BfSourceAlpha ||
				rs.blendDestination == BfOneMinusSourceAlpha
			)
				return PntScalar4;
		}
		if (rs.alphaTestEnable)
			return PntScalar4;
		if (rs.alphaToCoverageEnable)
			return PntScalar4;

		// Blend enable but not using alpha as a blend factor; determine
		// from write mask as if opaque.
		if (rs.colorWriteMask & CwAlpha)
			return PntScalar4;
		else if (rs.colorWriteMask & CwBlue)
			return PntScalar3;
		else if (rs.colorWriteMask & CwGreen)
			return PntScalar2;
		else if (rs.colorWriteMask & CwRed)
			return PntScalar1;
		else
			return PntVoid;
	}

	return PntScalar3;
}

int32_t PixelNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool PixelNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool PixelNodeTraits::evaluatePartial(
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

PinOrderType PixelNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	return PotConstant;
}

	}
}
