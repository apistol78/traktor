#ifndef traktor_render_TransformNodeTraits_H
#define traktor_render_TransformNodeTraits_H

#include "Render/Editor/Shader/INodeTraits.h"

namespace traktor
{
	namespace render
	{

class TransformNodeTraits : public INodeTraits
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getNodeTypes() const;

	virtual PinType getOutputPinType(
		const Node* node,
		const PinType* inputPinTypes,
		const OutputPin* outputPin
	) const;
	
	virtual PinType getAcceptableInputPinType(
		const Node* node,
		const InputPin* inputPin
	) const;
};

	}
}

#endif	// traktor_render_TransformNodeTraits_H
