#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/MatrixNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MatrixNodeTraits", 0, MatrixNodeTraits, INodeTraits)

TypeInfoSet MatrixNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MatrixIn >());
	typeSet.insert(&type_of< MatrixOut >());
	return typeSet;
}

PinType MatrixNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	if (is_a< MatrixIn >(node))
		return PntMatrix;
	else if (is_a< MatrixOut >(node))
		return PntScalar4;
	else
		return PntVoid;
}

PinType MatrixNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	if (is_a< MatrixIn >(node))
		return PntScalar4;
	else if (is_a< MatrixOut >(node))
		return PntMatrix;
	else
		return PntVoid;
}

	}
}
