#include "Core/Containers/SmallMap.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/ShaderGraphEvaluator.h"
#include "Render/Shader/Edge.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/ShaderGraphTraverse.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct EvaluateVisitor
{
	const ShaderGraph* shaderGraph;
	SmallMap< const OutputPin*, Constant > evaluatedConstants;
	AlignedVector< PinType > inputPinTypes;
	AlignedVector< Constant > inputConstants;

	bool operator () (Node* node)
	{
		const INodeTraits* nodeTraits = INodeTraits::find(node);
		T_ASSERT (nodeTraits);

		// Get node's input types and constants.
		int32_t inputPinCount = node->getInputPinCount();
		inputPinTypes.resize(inputPinCount);
		inputConstants.resize(inputPinCount);

		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			const OutputPin* outputPin = shaderGraph->findSourcePin(inputPin);
			if (outputPin)
			{
				inputPinTypes[i] = evaluatedConstants[outputPin].getType();
				inputConstants[i] = evaluatedConstants[outputPin];
			}
			else
			{
				inputPinTypes[i] = PntVoid;
				inputConstants[i] = Constant();
			}
		}

		// Convert constants into same widths.
		PinType intoPinType = PntVoid;
		for (int32_t i = 0; i < inputPinCount; ++i)
			intoPinType = std::max(inputConstants[i].getType(), intoPinType);
		for (int32_t i = 0; i < inputPinCount; ++i)
			inputConstants[i] = inputConstants[i].cast(intoPinType);

		// Evaluate output constants from input set.
		int32_t outputPinCount = node->getOutputPinCount();
		for (int32_t i = 0; i < outputPinCount; ++i)
		{
			const OutputPin* outputPin = node->getOutputPin(i);

			PinType outputPinType = nodeTraits->getOutputPinType(
				node,
				outputPin,
				inputPinTypes.c_ptr()
			);

			evaluatedConstants[outputPin] = Constant(outputPinType);

			nodeTraits->evaluatePartial(
				shaderGraph,
				node,
				outputPin,
				inputConstants.c_ptr(),
				evaluatedConstants[outputPin]
			);
		}

		return true;
	}

	bool operator () (Edge* edge)
	{
		return true;
	}

};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphEvaluator", ShaderGraphEvaluator, Object)

ShaderGraphEvaluator::ShaderGraphEvaluator(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
}

Constant ShaderGraphEvaluator::evaluate(const OutputPin* outputPin) const
{
	EvaluateVisitor visitor;
	visitor.shaderGraph = m_shaderGraph;
	ShaderGraphTraverse(m_shaderGraph, outputPin->getNode()).postorder(visitor);
	return visitor.evaluatedConstants[outputPin];
}

	}
}
