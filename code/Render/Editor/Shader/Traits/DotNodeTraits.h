/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_DotNodeTraits_H
#define traktor_render_DotNodeTraits_H

#include "Render/Editor/Shader/INodeTraits.h"

namespace traktor
{
	namespace render
	{

class DotNodeTraits : public INodeTraits
{
	T_RTTI_CLASS;

public:
	virtual TypeInfoSet getNodeTypes() const T_OVERRIDE T_FINAL;

	virtual bool isRoot(const ShaderGraph* shaderGraph, const Node* node) const T_OVERRIDE T_FINAL;

	virtual PinType getOutputPinType(
		const Node* node,
		const OutputPin* outputPin,
		const PinType* inputPinTypes
	) const T_OVERRIDE T_FINAL;
	
	virtual PinType getInputPinType(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const InputPin* inputPin,
		const PinType* inputPinTypes,
		const PinType* outputPinTypes
	) const T_OVERRIDE T_FINAL;

	virtual int32_t getInputPinGroup(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const InputPin* inputPin
	) const T_OVERRIDE T_FINAL;

	virtual bool evaluatePartial(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const OutputPin* nodeOutputPin,
		const Constant* inputConstants,
		Constant& outputConstant
	) const T_OVERRIDE T_FINAL;

	virtual bool evaluatePartial(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const OutputPin* nodeOutputPin,
		const OutputPin** inputOutputPins,
		const Constant* inputConstants,
		const OutputPin*& foldOutputPin
	) const T_OVERRIDE T_FINAL;

	virtual PinOrderType evaluateOrder(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const OutputPin* nodeOutputPin,
		const PinOrderType* inputPinOrders,
		bool frequentAsLinear
	) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_DotNodeTraits_H
