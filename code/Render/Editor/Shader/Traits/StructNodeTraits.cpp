/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Traits/StructNodeTraits.h"

namespace traktor::render
{
	namespace
	{

PinType pinTypeFromDataType(DataType dataType)
{
	switch (dataType)
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

	default:
		return PinType::Void;
	}
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.StructNodeTraits", 0, StructNodeTraits, INodeTraits)

TypeInfoSet StructNodeTraits::getNodeTypes() const
{
	return makeTypeInfoSet<
		ReadStruct,
		ReadStruct2
	>();
}

bool StructNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

bool StructNodeTraits::isInputTypeValid(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType pinType
) const
{
	if (inputPin->getName() == L"Struct")
		return isPinTypeStructBuffer(pinType);
	else
		return isPinTypeScalar(pinType);
}

PinType StructNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	const OutputPin* strctOutputPin = shaderGraph->findSourcePin(node->findInputPin(L"Struct"));
	if (!strctOutputPin)
		return PinType::Void;

	const Struct* strct = dynamic_type_cast< const Struct* >(strctOutputPin->getNode());
	if (!strct)
		return PinType::Void;

	if (const ReadStruct* readStruct = dynamic_type_cast< const ReadStruct * >(node))
	{
		auto elementName = readStruct->getName();
		auto elementType = strct->getElementType(elementName);
		return pinTypeFromDataType(elementType);
	}
	else if (const ReadStruct2* readStruct2 = dynamic_type_cast< const ReadStruct2* >(node))
	{
		auto elementName = outputPin->getName();
		auto elementType = strct->getElementType(elementName);
		return pinTypeFromDataType(elementType);
	}
	return PinType::Void;
}

PinType StructNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Buffer")
		return PinType::StructBuffer;
	else if (inputPin->getName() == L"Index")
		return PinType::Scalar1;
	else
		return PinType::Void;
}

int32_t StructNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return 0;
}

bool StructNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool StructNodeTraits::evaluatePartial(
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

PinOrder StructNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrder* inputPinOrders
) const
{
	return PinOrder::NonLinear;
}

}
