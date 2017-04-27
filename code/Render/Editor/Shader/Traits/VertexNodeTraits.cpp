/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/VertexNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.VertexNodeTraits", 0, VertexNodeTraits, INodeTraits)

TypeInfoSet VertexNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< VertexInput >());
	typeSet.insert(&type_of< VertexOutput >());
	return typeSet;
}

bool VertexNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return is_a< VertexOutput >(node);
}

PinType VertexNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (const VertexInput* vertexInputNode = dynamic_type_cast< const VertexInput* >(node))
	{
		DataType vertexDataType = vertexInputNode->getDataType();
		switch (vertexDataType)
		{
		case DtFloat1:
			return PntScalar1;

		case DtFloat2:
		case DtShort2:
		case DtShort2N:
		case DtHalf2:
			return PntScalar2;

		case DtFloat3:
			return PntScalar3;

		case DtFloat4:
		case DtByte4:
		case DtByte4N:
		case DtShort4:
		case DtShort4N:
		case DtHalf4:
			return PntScalar4;
		}
	}
	return PntVoid;
}

PinType VertexNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (is_a< VertexOutput >(node))
		return PntScalar4;
	else
		return PntVoid;
}

int32_t VertexNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return 0;
}

bool VertexNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool VertexNodeTraits::evaluatePartial(
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

PinOrderType VertexNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	return PotLinear;
}

	}
}
