/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cctype>
#include "Render/Shader/Script.h"
#include "Render/Editor/Shader/Traits/ScriptNodeTraits.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ScriptNodeTraits", 0, ScriptNodeTraits, INodeTraits)

TypeInfoSet ScriptNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Script >());
	return typeSet;
}

bool ScriptNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

PinType ScriptNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	T_ASSERT (is_a< Script >(node));
	T_ASSERT (outputPin->getNode() == node);

	const TypedOutputPin* typedOutputPin = static_cast< const TypedOutputPin* >(outputPin);
	switch (typedOutputPin->getType())
	{
	case PtScalar:
		return PntScalar1;
	case PtVector:
		return PntScalar4;
	case PtMatrix:
		return PntMatrix;
	case PtTexture2D:
		return PntTexture2D;
	case PtTexture3D:
		return PntTexture3D;
	case PtTextureCube:
		return PntTextureCube;
	default:
		return PntVoid;
	}
}

PinType ScriptNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	T_ASSERT (is_a< Script >(node));
	T_ASSERT (inputPin->getNode() == node);

	const TypedInputPin* typedInputPin = static_cast< const TypedInputPin* >(inputPin);
	switch (typedInputPin->getType())
	{
	case PtScalar:
		return PntScalar1;
	case PtVector:
		return PntScalar4;
	case PtMatrix:
		return PntMatrix;
	case PtTexture2D:
		return PntTexture2D;
	case PtTexture3D:
		return PntTexture3D;
	case PtTextureCube:
		return PntTextureCube;
	default:
		return PntVoid;
	}
}

int32_t ScriptNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool ScriptNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool ScriptNodeTraits::evaluatePartial(
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

PinOrderType ScriptNodeTraits::evaluateOrder(
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
