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
	std::map< const InputPin*, PinType >& m_inputPinTypes;
	std::map< const OutputPin*, PinType >& m_outputPinTypes;

	EvalInitialOutputTypes(
		const ShaderGraph* shaderGraph,
		std::map< const InputPin*, PinType >& inputPinTypes,
		std::map< const OutputPin*, PinType >& outputPinTypes
	)
	:	m_shaderGraph(shaderGraph)
	,	m_inputPinTypes(inputPinTypes)
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

	bool operator () (Edge* edge)
	{
		return true;
	}
};

struct EvalInputTypes
{
	const ShaderGraph* m_shaderGraph;
	std::map< const InputPin*, PinType >& m_inputPinTypes;
	std::map< const OutputPin*, PinType >& m_outputPinTypes;

	EvalInputTypes(
		const ShaderGraph* shaderGraph,
		std::map< const InputPin*, PinType >& inputPinTypes,
		std::map< const OutputPin*, PinType >& outputPinTypes
	)
	:	m_shaderGraph(shaderGraph)
	,	m_inputPinTypes(inputPinTypes)
	,	m_outputPinTypes(outputPinTypes)
	{
	}

	bool operator () (const Node* node)
	{
		const INodeTraits* nodeTraits = INodeTraits::find(node);
		T_ASSERT (nodeTraits);

		PinType outputPinTypes[16];

		uint32_t outputPinCount = node->getOutputPinCount();
		for (uint32_t i = 0; i < outputPinCount; ++i)
		{
			const OutputPin* outputPin = node->getOutputPin(i);

			T_ASSERT (m_outputPinTypes.find(outputPin) != m_outputPinTypes.end());
			outputPinTypes[i] = m_outputPinTypes[outputPin];
		}

		uint32_t inputPinCount = node->getInputPinCount();
		for (uint32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);

			T_ASSERT (m_inputPinTypes.find(inputPin) == m_inputPinTypes.end());
			m_inputPinTypes[inputPin] = nodeTraits->getInputPinType(m_shaderGraph, node, inputPin, outputPinTypes);

			if (m_inputPinTypes[inputPin] == PntVoid)
			{
				const OutputPin* sourceOutputPin = m_shaderGraph->findSourcePin(inputPin);
				if (sourceOutputPin)
					m_inputPinTypes[inputPin] = m_outputPinTypes[sourceOutputPin];
				else
					T_ASSERT (inputPin->isOptional());
			}
		}

		return true;
	}

	bool operator () (Edge* edge)
	{
		return true;
	}
};

struct EvalOutputTypes
{
	const ShaderGraph* m_shaderGraph;
	std::map< const InputPin*, PinType >& m_inputPinTypes;
	std::map< const OutputPin*, PinType >& m_outputPinTypes;
	uint32_t m_propagationCount;

	EvalOutputTypes(
		const ShaderGraph* shaderGraph,
		std::map< const InputPin*, PinType >& inputPinTypes,
		std::map< const OutputPin*, PinType >& outputPinTypes
	)
	:	m_shaderGraph(shaderGraph)
	,	m_inputPinTypes(inputPinTypes)
	,	m_outputPinTypes(outputPinTypes)
	,	m_propagationCount(0)
	{
	}

	bool operator () (const Node* node)
	{
		const INodeTraits* nodeTraits = INodeTraits::find(node);
		T_ASSERT (nodeTraits);

		uint32_t outputPinCount = node->getOutputPinCount();
		for (uint32_t i = 0; i < outputPinCount; ++i)
		{
			const OutputPin* outputPin = node->getOutputPin(i);

			std::vector< const InputPin* > destinationInputPins;
			m_shaderGraph->findDestinationPins(outputPin, destinationInputPins);

			PinType outputPinType = PntVoid;
			for (std::vector< const InputPin* >::const_iterator j = destinationInputPins.begin(); j != destinationInputPins.end(); ++j)
			{
				T_ASSERT (m_inputPinTypes.find(*j) != m_inputPinTypes.end());
				outputPinType = std::max(outputPinType, m_inputPinTypes[*j]);
			}

			T_ASSERT (m_outputPinTypes.find(outputPin) != m_outputPinTypes.end());
			if (m_outputPinTypes[outputPin] != outputPinType)
			{
				m_outputPinTypes[outputPin] = outputPinType;
				++m_propagationCount;
			}
		}

		return true;
	}

	bool operator () (Edge* edge)
	{
		return true;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphTypePropagation", ShaderGraphTypePropagation, Object)

ShaderGraphTypePropagation::ShaderGraphTypePropagation(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
	// Collect root nodes; assume all nodes with no output pins to be roots.
	RefArray< Node > roots;
	for (RefArray< Node >::const_iterator i = m_shaderGraph->getNodes().begin(); i != m_shaderGraph->getNodes().end(); ++i)
	{
		if ((*i)->getOutputPinCount() <= 0 && (*i)->getInputPinCount() > 0)
			roots.push_back(*i);
	}

	ShaderGraphTraverse traverse(m_shaderGraph, roots);

	// Initial estimate of output types.
	{
		EvalInitialOutputTypes visitor(m_shaderGraph, m_inputPinTypes, m_outputPinTypes);
		traverse.postorder(visitor);
	}

	for (;;)
	{
		m_inputPinTypes.clear();

		// Determine input types.
		{
			EvalInputTypes visitor(m_shaderGraph, m_inputPinTypes, m_outputPinTypes);
			traverse.preorder(visitor);
		}

		// Propagate input types as new output types.
		{
			EvalOutputTypes visitor(m_shaderGraph, m_inputPinTypes, m_outputPinTypes);
			traverse.preorder(visitor);
			if (!visitor.m_propagationCount)
				break;
		}
	}

	// Feed forward; determine output types from input types; some nodes have
	// fixed output types and we need to respect that.
	for (RefArray< Node >::const_iterator i = m_shaderGraph->getNodes().begin(); i != m_shaderGraph->getNodes().end(); ++i)
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
