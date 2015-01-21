#ifndef traktor_render_MixNodeTraits_H
#define traktor_render_MixNodeTraits_H

#include "Render/Editor/Shader/INodeTraits.h"

namespace traktor
{
	namespace render
	{

class MixNodeTraits : public INodeTraits
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getNodeTypes() const;

	virtual bool isRoot(const Node* node) const;

	virtual PinType getOutputPinType(
		const Node* node,
		const OutputPin* outputPin,
		const PinType* inputPinTypes
	) const;
	
	virtual PinType getInputPinType(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const InputPin* inputPin,
		const PinType* inputPinTypes,
		const PinType* outputPinTypes
	) const;

	virtual int32_t getInputPinGroup(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const InputPin* inputPin
	) const;

	virtual bool evaluatePartial(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const OutputPin* nodeOutputPin,
		const Constant* inputConstants,
		Constant& outputConstant
	) const;

	virtual bool evaluatePartial(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const OutputPin* nodeOutputPin,
		const OutputPin** inputOutputPins,
		const Constant* inputConstants,
		const OutputPin*& foldOutputPin
	) const;

	virtual PinOrderType evaluateOrder(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const OutputPin* nodeOutputPin,
		const PinOrderType* inputPinOrders,
		bool frequentAsLinear
	) const;
};

	}
}

#endif	// traktor_render_MixNodeTraits_H
