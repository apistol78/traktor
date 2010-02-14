#ifndef traktor_render_MatrixNodeTraits_H
#define traktor_render_MatrixNodeTraits_H

#include "Render/Editor/Shader/INodeTraits.h"

namespace traktor
{
	namespace render
	{

class MatrixNodeTraits : public INodeTraits
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

#endif	// traktor_render_MatrixNodeTraits_H
