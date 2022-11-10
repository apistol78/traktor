/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/VertexNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.VertexNodeTraits", 0, VertexNodeTraits, INodeTraits)

TypeInfoSet VertexNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< VertexInput >();
	typeSet.insert< VertexOutput >();
	return typeSet;
}

bool VertexNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return is_a< VertexOutput >(node);
}

PinType VertexNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
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
		case DtInteger1:
			return PinType::Scalar1;

		case DtFloat2:
		case DtShort2:
		case DtShort2N:
		case DtHalf2:
		case DtInteger2:
			return PinType::Scalar2;

		case DtFloat3:
		case DtInteger3:
			return PinType::Scalar3;

		case DtFloat4:
		case DtByte4:
		case DtByte4N:
		case DtShort4:
		case DtShort4N:
		case DtHalf4:
		case DtInteger4:
			return PinType::Scalar4;
		}
	}
	return PinType::Void;
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
		return PinType::Scalar4;
	else
		return PinType::Void;
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
