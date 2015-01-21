#include <cctype>
#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/SwizzleNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SwizzleNodeTraits", 0, SwizzleNodeTraits, INodeTraits)

TypeInfoSet SwizzleNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Swizzle >());
	return typeSet;
}

bool SwizzleNodeTraits::isRoot(const Node* node) const
{
	return false;
}

PinType SwizzleNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	const std::wstring& pattern = checked_type_cast< const Swizzle* >(node)->get();
	switch (pattern.length())
	{
	case 1:
		return PntScalar1;
	case 2:
		return PntScalar2;
	case 3:
		return PntScalar3;
	case 4:
		return PntScalar4;
	default:
		return PntVoid;
	}
}

PinType SwizzleNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* inputPinTypes,
	const PinType* outputPinTypes
) const
{
	const std::wstring& pattern = checked_type_cast< const Swizzle* >(node)->get();
	
	PinType inputPinType = PntVoid;
	for (size_t i = 0; i < pattern.length(); ++i)
	{
		switch (std::tolower(pattern[i]))
		{
		case L'x':
			inputPinType = std::max< PinType >(inputPinType, PntScalar1);
			break;
		case L'y':
			inputPinType = std::max< PinType >(inputPinType, PntScalar2);
			break;
		case L'z':
			inputPinType = std::max< PinType >(inputPinType, PntScalar3);
			break;
		case L'w':
			inputPinType = std::max< PinType >(inputPinType, PntScalar4);
			break;
		}
	}

	return inputPinType;
}

int32_t SwizzleNodeTraits::getInputPinGroup(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin
) const
{
	return 0;
}

bool SwizzleNodeTraits::evaluatePartial(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	const std::wstring& pattern = checked_type_cast< const Swizzle* >(node)->get();

	for (size_t i = 0; i < pattern.length(); ++i)
	{
		switch (std::tolower(pattern[i]))
		{
		case L'x':
			if (inputConstants[0].isConstX())
				outputConstant.setValue(i, inputConstants[0].x());
			else
				outputConstant.setVariant(i);
			break;
		case L'y':
			if (inputConstants[0].isConstY())
				outputConstant.setValue(i, inputConstants[0].y());
			else
				outputConstant.setVariant(i);
			break;
		case L'z':
			if (inputConstants[0].isConstZ())
				outputConstant.setValue(i, inputConstants[0].z());
			else
				outputConstant.setVariant(i);
			break;
		case L'w':
			if (inputConstants[0].isConstW())
				outputConstant.setValue(i, inputConstants[0].w());
			else
				outputConstant.setVariant(i);
			break;
		case L'0':
			outputConstant.setValue(i, 0.0f);
			break;
		case L'1':
			outputConstant.setValue(i, 1.0f);
			break;
		}
	}

	return true;
}

bool SwizzleNodeTraits::evaluatePartial(
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

PinOrderType SwizzleNodeTraits::evaluateOrder(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* nodeOutputPin,
	const PinOrderType* inputPinOrders,
	bool frequentAsLinear
) const
{
	return inputPinOrders[0];
}

	}
}
