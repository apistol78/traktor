#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Traits/SamplerNodeTraits.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SamplerNodeTraits", 0, SamplerNodeTraits, INodeTraits)

TypeInfoSet SamplerNodeTraits::getNodeTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Sampler >());
	return typeSet;
}

PinType SamplerNodeTraits::getOutputPinType(
	const Node* node,
	const OutputPin* outputPin,
	const PinType* inputPinTypes
) const
{
	return PntScalar4;
}

PinType SamplerNodeTraits::getInputPinType(
	const ShaderGraph* shaderGraph,
	const Node* node,
	const InputPin* inputPin,
	const PinType* outputPinTypes
) const
{
	if (inputPin->getName() == L"Texture")
		return PntTexture;
	else
	{
		switch (checked_type_cast< const Sampler* >(node)->getLookup())
		{
		case Sampler::LuSimple:
			return PntScalar2;
		case Sampler::LuCube:
			return PntScalar3;
		case Sampler::LuVolume:
			return PntScalar3;
		default:
			return PntVoid;
		}
	}
}

	}
}
