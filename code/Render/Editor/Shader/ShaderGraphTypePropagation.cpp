#include "Core/Log/Log.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/ShaderGraphTypePropagation.h"
#include "Render/Editor/Shader/ShaderGraphUtilities.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct EvalInitialOutputTypes
{
	const ShaderGraph* m_shaderGraph;
	std::map< const OutputPin*, PinType >& m_outputPinTypes;

	EvalInitialOutputTypes(
		const ShaderGraph* shaderGraph,
		std::map< const OutputPin*, PinType >& outputPinTypes
	)
	:	m_shaderGraph(shaderGraph)
	,	m_outputPinTypes(outputPinTypes)
	{
	}

	bool operator () (const Node* node)
	{
		const INodeTraits* nodeTraits = INodeTraits::find(node);
		T_ASSERT (nodeTraits);

		PinType inputPinTypes[16];

		uint32_t inputPinCount = node->getInputPinCount();
		for (uint32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			T_ASSERT (inputPin);

			const OutputPin* sourceOutputPin = m_shaderGraph->findSourcePin(inputPin);
			if (!sourceOutputPin)
			{
				T_ASSERT (inputPin->isOptional());
				continue;
			}

			inputPinTypes[i] = m_outputPinTypes[sourceOutputPin];
		}

		uint32_t outputPinCount = node->getOutputPinCount();
		for (uint32_t i = 0; i < outputPinCount; ++i)
		{
			const OutputPin* outputPin = node->getOutputPin(i);
			m_outputPinTypes[outputPin] = nodeTraits->getOutputPinType(node, outputPin, inputPinTypes);
		}

		return true;
	}

	bool operator () (const Edge* edge)
	{
		return true;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphTypePropagation", ShaderGraphTypePropagation, Object)

ShaderGraphTypePropagation::ShaderGraphTypePropagation(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
	const RefArray< Node >& nodes = m_shaderGraph->getNodes();

	// Initial estimate of output types.
	{
		RefArray< Node > roots;
		for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			if ((*i)->getOutputPinCount() <= 0 && (*i)->getInputPinCount() > 0)
				roots.push_back(*i);
		}

		ShaderGraphTraverse traverse(m_shaderGraph, roots);
		EvalInitialOutputTypes visitor(m_shaderGraph, m_outputPinTypes);
		traverse.postorder(visitor);
	}

	RefSet< const Node > nodeSetInput, nodeSetOutput;

	// Initial set of input nodes.
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		nodeSetInput.insert(*i);
	
	// Iteratively solve types until all types are stable.
	uint32_t iterationCount = 0;
	for (;;)
	{
		nodeSetOutput.clear();

		// Determine input types.
		for (RefSet< const Node >::const_iterator i = nodeSetInput.begin(); i != nodeSetInput.end(); ++i)
		{
			const Node* node = *i;
			T_ASSERT (node);

			const INodeTraits* nodeTraits = INodeTraits::find(node);
			T_ASSERT (nodeTraits);

			PinType outputPinTypes[4];

			uint32_t outputPinCount = node->getOutputPinCount();
			T_ASSERT (outputPinCount <= sizeof_array(outputPinTypes));

			for (uint32_t j = 0; j < outputPinCount; ++j)
			{
				const OutputPin* outputPin = node->getOutputPin(j);
				T_ASSERT (m_outputPinTypes.find(outputPin) != m_outputPinTypes.end());
				
				outputPinTypes[j] = m_outputPinTypes[outputPin];
			}

			uint32_t inputPinCount = node->getInputPinCount();
			for (uint32_t j = 0; j < inputPinCount; ++j)
			{
				const InputPin* inputPin = node->getInputPin(j);
				T_ASSERT (inputPin);

				PinType inputPinType = nodeTraits->getInputPinType(m_shaderGraph, node, inputPin, outputPinTypes);
				if (inputPinType == PntVoid)
				{
					const OutputPin* sourceOutputPin = m_shaderGraph->findSourcePin(inputPin);
					if (sourceOutputPin)
						inputPinType = m_outputPinTypes[sourceOutputPin];
					else
						T_ASSERT (inputPin->isOptional());
				}

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

		// We're finished when we havn't modified a single input pin type.
		if (nodeSetOutput.empty())
			break;

		// Propagate input types as new output types.
		for (RefSet< const Node >::const_iterator i = nodeSetOutput.begin(); i != nodeSetOutput.end(); ++i)
		{
			const Node* node = *i;
			T_ASSERT (node);

			const INodeTraits* nodeTraits = INodeTraits::find(node);
			T_ASSERT (nodeTraits);

			uint32_t outputPinCount = node->getOutputPinCount();
			for (uint32_t j = 0; j < outputPinCount; ++j)
			{
				const OutputPin* outputPin = node->getOutputPin(j);
				T_ASSERT (outputPin);

				std::vector< const InputPin* > destinationInputPins;
				m_shaderGraph->findDestinationPins(outputPin, destinationInputPins);

				PinType outputPinType = PntVoid;
				for (std::vector< const InputPin* >::const_iterator k = destinationInputPins.begin(); k != destinationInputPins.end(); ++k)
				{
					T_ASSERT (m_inputPinTypes.find(*k) != m_inputPinTypes.end());
					outputPinType = std::max(outputPinType, m_inputPinTypes[*k]);
				}

				T_ASSERT (m_outputPinTypes.find(outputPin) != m_outputPinTypes.end());
				if (outputPinType < m_outputPinTypes[outputPin])
				{
					nodeSetInput.insert(node);
					m_outputPinTypes[outputPin] = outputPinType;
				}
			}
		}

		++iterationCount;
	}
	
	log::debug << L"Type propagation solved in " << iterationCount << L" iteration(s)" << Endl;

	// Feed forward; determine output types from input types; some nodes have
	// fixed output types and we need to respect that.
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		const INodeTraits* nodeTraits = INodeTraits::find(*i);
		T_ASSERT (nodeTraits);

		uint32_t inputPinCount = (*i)->getInputPinCount();
		uint32_t outputPinCount = (*i)->getOutputPinCount();

		std::vector< PinType > inputPinTypes(inputPinCount);
		for (uint32_t j = 0; j < inputPinCount; ++j)
		{
			const InputPin* inputPin = (*i)->getInputPin(j);
			inputPinTypes[j] = m_inputPinTypes[inputPin];
		}

		for (uint32_t j = 0; j < outputPinCount; ++j)
		{
			const OutputPin* outputPin = (*i)->getOutputPin(j);
			m_outputPinTypes[outputPin] = nodeTraits->getOutputPinType(
				*i,
				outputPin,
				inputPinCount > 0 ? &inputPinTypes[0] : 0
			);
		}
	}
}

PinType ShaderGraphTypePropagation::evaluate(const InputPin* inputPin) const
{
	std::map< const InputPin*, PinType >::const_iterator i = m_inputPinTypes.find(inputPin);
	T_ASSERT (i != m_inputPinTypes.end());
	return i != m_inputPinTypes.end() ? i->second : PntVoid;
}

PinType ShaderGraphTypePropagation::evaluate(const OutputPin* outputPin) const
{
	std::map< const OutputPin*, PinType >::const_iterator i = m_outputPinTypes.find(outputPin);
	T_ASSERT (i != m_outputPinTypes.end());
	return i != m_outputPinTypes.end() ? i->second : PntVoid;
}

void ShaderGraphTypePropagation::set(const InputPin* inputPin, PinType inputPinType)
{
	T_ASSERT (inputPinType != PntVoid);
	m_inputPinTypes[inputPin] = inputPinType;
}

void ShaderGraphTypePropagation::set(const OutputPin* outputPin, PinType outputPinType)
{
	T_ASSERT (outputPinType != PntVoid);
	m_outputPinTypes[outputPin] = outputPinType;
}

	}
}
