/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Render/Editor/Shader/Constant.h"
#include "Render/Editor/Shader/PinType.h"

namespace traktor
{
	namespace render
	{

class InputPin;
class Node;
class OutputPin;
class ShaderGraph;

/*! Shader graph node traits.
 * \ingroup Render
 *
 * Node traits define behavior of nodes
 * such as type conversions etc.
 *
 * These are used by optimizing steps in
 * order to have those steps independent
 * of node types.
 */
class INodeTraits : public Object
{
	T_RTTI_CLASS;

public:
	/*! Get set of types the trait recognizes.
	 */
	virtual TypeInfoSet getNodeTypes() const = 0;

	/*! Return true if node should be considered a root.
	 */
	virtual bool isRoot(const ShaderGraph* shaderGraph, const Node* node) const = 0;

	/*! Determine type of output pin from given types of all input pins.
	 */
	virtual PinType getOutputPinType(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const OutputPin* outputPin,
		const PinType* inputPinTypes
	) const = 0;

	/*! Get type of input pin.
	 */
	virtual PinType getInputPinType(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const InputPin* inputPin,
		const PinType* inputPinTypes,
		const PinType* outputPinTypes
	) const = 0;

	/*! Get input pin independence group.
	 *
	 * A independence group is defined as being a number
	 * in which input pins can be swapped without
	 * changing the result of the node.
	 * For example both input pins of an Add node
	 * can be swapped without changing the outcome.
	 */
	virtual int32_t getInputPinGroup(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const InputPin* inputPin
	) const = 0;

	/*! Evaluate output pin from partially constant input set.
	 *
	 * \return True if output has been evaluated.
	 */
	virtual bool evaluatePartial(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const OutputPin* nodeOutputPin,
		const Constant* inputConstants,
		Constant& outputConstant
	) const = 0;

	/*! Evaluate rewire output pin from partially constant input set.
	 *
	 * \return True if a folding output pin has been determined.
	 */
	virtual bool evaluatePartial(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const OutputPin* nodeOutputPin,
		const OutputPin** inputOutputPins,
		const Constant* inputConstants,
		const OutputPin*& foldOutputPin
	) const = 0;

	/*! Evaluate order of output pin.
	 *
	 * \return Mathematical order of output pin; constant, linear or exponential.
	 */
	virtual PinOrderType evaluateOrder(
		const ShaderGraph* shaderGraph,
		const Node* node,
		const OutputPin* nodeOutputPin,
		const PinOrderType* inputPinOrders,
		bool frequentAsLinear
	) const = 0;

	/*! Get node traits.
	 */
	static const INodeTraits* find(const Node* node);
};

	}
}

