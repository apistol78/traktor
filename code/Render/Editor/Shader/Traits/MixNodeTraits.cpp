#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Traits/MixNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MixNodeTraits", 0, MixNodeTraits, INodeTraits)

TypeInfoSet MixNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MixIn >());
	typeSet.insert(&type_of< MixOut >());
	return typeSet;
}

PinType MixNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (is_a< MixIn >(node))
		return PntScalar4;
	else if (is_a< MixOut >(node))
		return PntScalar1;
	else
		return PntVoid;
}

PinType MixNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
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

bool MixNodeTraits::evaluate(
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
