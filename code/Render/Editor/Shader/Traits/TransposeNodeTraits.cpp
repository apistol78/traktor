#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/TransposeNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TransposeNodeTraits", 0, TransposeNodeTraits, INodeTraits)

TypeInfoSet TransposeNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Transpose >());
	return typeSet;
}

PinType TransposeNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return PntMatrix;
}

PinType TransposeNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	return PntMatrix;
}

	}
}
