#include "Render/Editor/Shader/Nodes.h"
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
	typeSet.insert< IndexedUniform >();
	typeSet.insert< Uniform >();
	return typeSet;
}

bool UniformNodeTraits::isRoot(const ShaderGraph* shaderGraph, const Node* node) const
{
	return false;
}

PinType UniformNodeTraits::getOutputPinType(
	const ShaderGraph* shaderGraph,
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
	case ParameterType::Scalar:
		return PntScalar1;
	case ParameterType::Vector:
		return PntScalar4;
	case ParameterType::Matrix:
		return PntMatrix;
	case ParameterType::Texture2D:
		return PntTexture2D;
	case ParameterType::Texture3D:
		return PntTexture3D;
	case ParameterType::TextureCube:
		return PntTextureCube;
	case ParameterType::Image2D:
		return PntImage2D;
	case ParameterType::Image3D:
		return PntImage3D;
	case ParameterType::ImageCube:
		return PntImageCube;
	case ParameterType::StructBuffer:
		return PntStructBuffer;
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
		if (inputPin->getName() == L"Initial")
		{
			switch (uniformNode->getParameterType())
			{
			case ParameterType::Scalar:
				return PntScalar1;
			case ParameterType::Vector:
				return PntScalar4;
			case ParameterType::Matrix:
				return PntMatrix;
			case ParameterType::Texture2D:
				return PntTexture2D;
			case ParameterType::Texture3D:
				return PntTexture3D;
			case ParameterType::TextureCube:
				return PntTextureCube;
			case ParameterType::Image2D:
				return PntImage2D;
			case ParameterType::Image3D:
				return PntImage3D;
			case ParameterType::ImageCube:
				return PntImageCube;
			case ParameterType::StructBuffer:
				return PntStructBuffer;
			default:
				return PntVoid;
			}
		}
		else
			return PntVoid;
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
				indexedUniform->getParameterType() < ParameterType::Texture2D &&
				indexedUniform->getFrequency() >= UpdateFrequency::Draw
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
			uniform->getParameterType() < ParameterType::Texture2D &&
			uniform->getFrequency() >= UpdateFrequency::Draw
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
