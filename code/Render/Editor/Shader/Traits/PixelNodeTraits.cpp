#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/PixelNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.PixelNodeTraits", 0, PixelNodeTraits, INodeTraits)

TypeInfoSet PixelNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PixelOutput >());
	return typeSet;
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
	const PinType* outputPinTypes
) const
{
	const PixelOutput* pixelOutputNode = checked_type_cast< const PixelOutput* >(node);

	if (!pixelOutputNode->getBlendEnable() && !pixelOutputNode->getAlphaTestEnable())
	{
		uint32_t writeMask = pixelOutputNode->getColorWriteMask();
		if (writeMask & PixelOutput::CwAlpha)
			return PntScalar4;
		else if (writeMask & PixelOutput::CwBlue)
			return PntScalar3;
		else if (writeMask & PixelOutput::CwGreen)
			return PntScalar2;
		else if (writeMask & PixelOutput::CwRed)
			return PntScalar1;
		else
			return PntVoid;
	}
	else
	{
		if (pixelOutputNode->getBlendEnable())
		{
			if (
				pixelOutputNode->getBlendSource() == PixelOutput::BfSourceAlpha ||
				pixelOutputNode->getBlendSource() == PixelOutput::BfOneMinusSourceAlpha ||
				pixelOutputNode->getBlendDestination() == PixelOutput::BfSourceAlpha ||
				pixelOutputNode->getBlendDestination() == PixelOutput::BfOneMinusSourceAlpha
			)
				return PntScalar4;
		}
		if (pixelOutputNode->getAlphaTestEnable())
			return PntScalar4;
	}

	return PntScalar3;
}

bool PixelNodeTraits::evaluate(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const OutputPin* outputPin,
	const Constant* inputConstants,
	Constant& outputConstant
) const
{
	return false;
}

	}
}
