#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/TransformNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TransformNodeTraits", 0, TransformNodeTraits, INodeTraits)

TypeInfoSet TransformNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Transform >());
	return typeSet;
}

PinType TransformNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return PntScalar4;
}

PinType TransformNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Input")
		return PntScalar4;
	else
		return PntMatrix;
}

bool TransformNodeTraits::evaluate(
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
