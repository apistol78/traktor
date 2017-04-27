/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/UniformNodeTraits.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.UniformNodeTraits", 0, UniformNodeTraits, INodeTraits)

TypeInfoSet UniformNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IndexedUniform >());
	typeSet.insert(&type_of< Uniform >());
	return typeSet;
}

bool UniformNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

PinType UniformNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	ParameterType parameterType;

	if (const IndexedUniform* indexedUniformNode = dynamic_type_cast< const IndexedUniform* >(node))
		parameterType = indexedUniformNode->getParameterType();
	else if (const Uniform* uniformNode = dynamic_type_cast< const Uniform* >(node))
		parameterType = uniformNode->getParameterType();
	else
		return PntVoid;

	switch (parameterType)
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

PinType UniformNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	if (is_a< IndexedUniform >(node))
		return PntScalar1;
	else if (const Uniform* uniformNode = dynamic_type_cast< const Uniform* >(node))
	{
		switch (uniformNode->getParameterType())
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
	else
		return PntVoid;
}

int32_t UniformNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return getInputPinIndex(node, inputPin);
}

bool UniformNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

bool UniformNodeTraits::evaluatePartial(
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

PinOrderType UniformNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	if (const IndexedUniform* indexedUniform = dynamic_type_cast< const IndexedUniform* >(node))
	{
		if (inputPinOrders[0] == PotConstant)
		{
			if (
				frequentAsLinear &&
				indexedUniform->getParameterType() < PtTexture2D &&
				indexedUniform->getFrequency() >= UfDraw
			)
				return PotLinear;
			else
				return PotConstant;
		}
		else
			return PotNonLinear;
	}
	else if (const Uniform* uniform = dynamic_type_cast< const Uniform* >(node))
	{
		if (
			frequentAsLinear &&
			uniform->getParameterType() < PtTexture2D &&
			uniform->getFrequency() >= UfDraw
		)
			return PotLinear;
		else
			return PotConstant;
	}
	else
		return PotConstant;
}

	}
}
