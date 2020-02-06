#include "Core/Log/Log.h"
#include "Render/Editor/GraphTraverse.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/ShaderGraphTypePropagation.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphTypePropagation", ShaderGraphTypePropagation, Object)

ShaderGraphTypePropagation::ShaderGraphTypePropagation(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
	const RefArray< Node >& nodes = m_shaderGraph->getNodes();

	// Initial estimate of output types.
	{
		RefArray< Node > roots;
		for (auto node : nodes)
		{
			if (node->getOutputPinCount() <= 0 && node->getInputPinCount() > 0)
				roots.push_back(node);
		}

		GraphTraverse traverse(m_shaderGraph, roots);
		traverse.postorder([&](const Node* node) {
			const INodeTraits* nodeTraits = INodeTraits::find(node);
			if (!nodeTraits)
				return true;

			PinType inputPinTypes[16];

			uint32_t inputPinCount = node->getInputPinCount();
			for (uint32_t i = 0; i < inputPinCount; ++i)
			{
				const InputPin* inputPin = node->getInputPin(i);
				T_ASSERT(inputPin);

				const OutputPin* sourceOutputPin = m_shaderGraph->findSourcePin(inputPin);
				if (!sourceOutputPin)
				{
					if (!inputPin->isOptional())
						log::warning << L"Mandatory input pin \"" << inputPin->getName() << L"\" of node " << node->getId().format() << L" (" << type_name(node) << L") not connected." << Endl;

					inputPinTypes[i] = PntVoid;
					continue;
				}

				inputPinTypes[i] = m_outputPinTypes[sourceOutputPin];
			}

			uint32_t outputPinCount = node->getOutputPinCount();
			for (uint32_t i = 0; i < outputPinCount; ++i)
			{
				const OutputPin* outputPin = node->getOutputPin(i);
				m_outputPinTypes[outputPin] = nodeTraits->getOutputPinType(m_shaderGraph, node, outputPin, inputPinTypes);
			}

			return true;
		});
	}

	RefSet< const Node > nodeSetInput, nodeSetOutput;

	// Initial set of input nodes.
	for (auto node : nodes)
		nodeSetInput.insert(node);

	// Iteratively solve types until all types are stable.
	PinType currentInputPinTypes[32];
	PinType outputPinTypes[32];
	uint32_t iterationCount = 0;
	for (;;)
	{
		nodeSetOutput.clear();

		// Determine input types.
		for (const auto node : nodeSetInput)
		{
			const INodeTraits* nodeTraits = INodeTraits::find(node);
			T_ASSERT(nodeTraits);

			uint32_t inputPinCount = node->getInputPinCount();
			T_ASSERT(inputPinCount < sizeof_array(currentInputPinTypes));

			uint32_t outputPinCount = node->getOutputPinCount();
			T_ASSERT(outputPinCount < sizeof_array(outputPinTypes));

			// Get current set of types for node's inputs.
			for (uint32_t i = 0; i < inputPinCount; ++i)
			{
				const InputPin* inputPin = node->getInputPin(i);
				T_ASSERT(inputPin);

				const OutputPin* sourceOutputPin = m_shaderGraph->findSourcePin(inputPin);
				if (sourceOutputPin)
					currentInputPinTypes[i] = m_outputPinTypes[sourceOutputPin];
				else
				{
					T_ASSERT(inputPin->isOptional());
					currentInputPinTypes[i] = PntVoid;
				}
			}

			// Get set of output types for node's outputs.
			for (uint32_t i = 0; i < outputPinCount; ++i)
			{
				const OutputPin* outputPin = node->getOutputPin(i);
				T_ASSERT(m_outputPinTypes.find(outputPin) != m_outputPinTypes.end());

				outputPinTypes[i] = m_outputPinTypes[outputPin];
			}

			// Evaluate possible new input type from type sets.
			for (uint32_t i = 0; i < inputPinCount; ++i)
			{
				const InputPin* inputPin = node->getInputPin(i);
				T_ASSERT(inputPin);

				PinType inputPinType = nodeTraits->getInputPinType(m_shaderGraph, node, inputPin, currentInputPinTypes, outputPinTypes);
				if (inputPinType == PntVoid)
					inputPinType = currentInputPinTypes[i];

				if (m_inputPinTypes[inputPin] != inputPinType)
				{
					const OutputPin* sourceOutputPin = m_shaderGraph->findSourcePin(inputPin);
					if (sourceOutputPin)
						nodeSetOutput.insert(sourceOutputPin->getNode());

					m_inputPinTypes[inputPin] = inputPinType;
				}
			}
		}

		nodeSetInput.clear();

		// We're finished when we haven't modified a single input pin type.
		if (nodeSetOutput.empty())
			break;

		// Propagate input types as new output types.
		for (const auto node : nodeSetOutput)
		{
			uint32_t outputPinCount = node->getOutputPinCount();
			for (uint32_t i = 0; i < outputPinCount; ++i)
			{
				const OutputPin* outputPin = node->getOutputPin(i);
				T_ASSERT(outputPin);

				AlignedVector< const InputPin* > destinationInputPins;
				m_shaderGraph->findDestinationPins(outputPin, destinationInputPins);

				PinType outputPinType = PntVoid;
				for (const auto destinationInputPin : destinationInputPins)
				{
					T_ASSERT(m_inputPinTypes.find(destinationInputPin) != m_inputPinTypes.end());
					outputPinType = std::max(outputPinType, m_inputPinTypes[destinationInputPin]);
				}

				T_ASSERT(m_outputPinTypes.find(outputPin) != m_outputPinTypes.end());
				if (outputPinType < m_outputPinTypes[outputPin])
				{
					nodeSetInput.insert(node);
					m_outputPinTypes[outputPin] = outputPinType;
				}
			}
		}

		++iterationCount;
	}

	// Finally ensure output nodes have correct output types; thus they cannot permutate.
	for (const auto node : nodes)
	{
		if (node->getOutputPinCount() > 0 && node->getInputPinCount() == 0)
		{
			const INodeTraits* nodeTraits = INodeTraits::find(node);
			T_ASSERT(nodeTraits);

			uint32_t outputPinCount = node->getOutputPinCount();
			for (uint32_t j = 0; j < outputPinCount; ++j)
			{
				const OutputPin* outputPin = node->getOutputPin(j);
				T_ASSERT(outputPin);

				m_outputPinTypes[outputPin] = nodeTraits->getOutputPinType(m_shaderGraph, node, outputPin, 0);
			}
		}
	}

	T_DEBUG(L"Type propagation solved in " << iterationCount << L" iteration(s)");
}

PinType ShaderGraphTypePropagation::evaluate(const InputPin* inputPin) const
{
	SmallMap< const InputPin*, PinType >::const_iterator i = m_inputPinTypes.find(inputPin);
	T_ASSERT(i != m_inputPinTypes.end());
	return i != m_inputPinTypes.end() ? i->second : PntVoid;
}

PinType ShaderGraphTypePropagation::evaluate(const OutputPin* outputPin) const
{
	SmallMap< const OutputPin*, PinType >::const_iterator i = m_outputPinTypes.find(outputPin);
	T_ASSERT(i != m_outputPinTypes.end());
	return i != m_outputPinTypes.end() ? i->second : PntVoid;
}

void ShaderGraphTypePropagation::set(const InputPin* inputPin, PinType inputPinType)
{
	T_ASSERT(inputPinType != PntVoid);
	m_inputPinTypes[inputPin] = inputPinType;
}

void ShaderGraphTypePropagation::set(const OutputPin* outputPin, PinType outputPinType)
{
	T_ASSERT(outputPinType != PntVoid);
	m_outputPinTypes[outputPin] = outputPinType;
}

	}
}
