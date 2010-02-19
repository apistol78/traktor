#ifndef traktor_render_SwizzleNodeTraits_H
#define traktor_render_SwizzleNodeTraits_H

#include "Render/Editor/Shader/INodeTraits.h"

namespace traktor
{
	namespace render
	{

class SwizzleNodeTraits : public INodeTraits
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getNodeTypes() const;

	virtual PinType getOutputPinType(
		const Node* node,
		const OutputPin* outputPin,
		const PinType* inputPinTypes
	) const;
	
	virtual PinType getInputPinType(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const InputPin* inputPin,
		const PinType* outputPinTypes
	) const;
};

	}
}

#endif	// traktor_render_SwizzleNodeTraits_H
