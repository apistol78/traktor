/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Traits/MixNodeTraits.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MixNodeTraits", 0, MixNodeTraits, INodeTraits)

TypeInfoSet MixNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MixIn >());
	typeSet.insert(&type_of< MixOut >());
	return typeSet;
}

bool MixNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

PinType MixNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (is_a< MixIn >(node))
	{
		if (inputPinTypes[1] == PntVoid && inputPinTypes[2] == PntVoid && inputPinTypes[3] == PntVoid)
			return PntScalar1;
		else if (inputPinTypes[2] == PntVoid && inputPinTypes[3] == PntVoid)
			return PntScalar2;
		else if (inputPinTypes[3] == PntVoid)
			return PntScalar3;
		else
			return PntScalar4;
	}
	else if (is_a< MixOut >(node))
		return PntScalar1;
	else
		return PntVoid;
}

PinType MixNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (is_a< MixIn >(node))
		return PntScalar1;
	else if (const MixOut* mixOutNode = dynamic_type_cast< const MixOut* >(node))
	{
		if (shaderGraph->getDestinationCount(node->getOutputPin(3)) > 0)
			return PntScalar4;
		
		if (shaderGraph->getDestinationCount(node->getOutputPin(2)) > 0)
			return PntScalar3;

		if (shaderGraph->getDestinationCount(node->getOutputPin(1)) > 0)
			return PntScalar2;

		if (shaderGraph->getDestinationCount(node->getOutputPin(0)) > 0)
			return PntScalar1;

		return PntVoid;
	}
	else
		return PntVoid;
}

int32_t MixNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool MixNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	if (is_a< MixIn >(node))
	{
		for (int32_t i = 0; i < outputConstant.getWidth(); ++i)
		{
			if (inputConstants[i].isConst(0))
				outputConstant.setValue(i, inputConstants[i].getValue(0));
			else
				outputConstant.setVariant(i);
		}
		return true;
	}
	else if (is_a< MixOut >(node))
	{
		if (nodeOutputPin->getName() == L"X")
		{
			if (inputConstants[0].isConstX())
				outputConstant.setValue(0, inputConstants[0].x());
			else
				outputConstant.setVariant(0);
		}
		else if (nodeOutputPin->getName() == L"Y")
		{
			if (inputConstants[0].isConstY())
				outputConstant.setValue(1, inputConstants[0].y());
			else
				outputConstant.setVariant(1);
		}
		else if (nodeOutputPin->getName() == L"Z")
		{
			if (inputConstants[0].isConstZ())
				outputConstant.setValue(2, inputConstants[0].z());
			else
				outputConstant.setVariant(2);
		}
		else if (nodeOutputPin->getName() == L"W")
		{
			if (inputConstants[0].isConstW())
				outputConstant.setValue(3, inputConstants[0].w());
			else
				outputConstant.setVariant(3);
		}
		else
			return false;

		return true;
	}
	return false;
}

bool MixNodeTraits::evaluatePartial(
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

PinOrderType MixNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	return pinOrderMax(inputPinOrders, node->getInputPinCount());
}

	}
}
