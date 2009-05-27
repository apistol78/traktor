#include <set>
#include <stack>
#include "Render/Editor/Shader/ShaderGraphCombinations.h"
#include "Render/ShaderGraph.h"
#include "Render/ShaderGraphAdjacency.h"
#include "Render/Nodes.h"
#include "Render/Edge.h"
#include "Core/Serialization/DeepClone.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphCombinations", ShaderGraphCombinations, Object)

ShaderGraphCombinations::ShaderGraphCombinations(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
,	m_shaderGraphAdj(gc_new< ShaderGraphAdjacency >(shaderGraph))
{
	RefArray< Branch > branchNodes;
	m_shaderGraph->findNodesOf< Branch >(branchNodes);

	std::set< std::wstring > parameterNames;
	for (RefArray< Branch >::iterator i = branchNodes.begin(); i != branchNodes.end(); ++i)
	{
		std::wstring name = (*i)->getParameterName();
		if (parameterNames.find(name) == parameterNames.end())
		{
			m_parameterNames.push_back(name);
			parameterNames.insert(name);
		}
	}
}

const std::vector< std::wstring >& ShaderGraphCombinations::getParameterNames() const
{
	return m_parameterNames;
}

uint32_t ShaderGraphCombinations::getCombinationCount() const
{
	return 1 << uint32_t(m_parameterNames.size());
}

std::vector< std::wstring > ShaderGraphCombinations::getParameterCombination(uint32_t combination) const
{
	std::vector< std::wstring > parameterCombination;
	for (uint32_t i = 0; i < uint32_t(m_parameterNames.size()); ++i)
	{
		if ((combination & (1 << i)) != 0)
			parameterCombination.push_back(m_parameterNames[i]);
	}
	return parameterCombination;
}

ShaderGraph* ShaderGraphCombinations::generate(uint32_t combination) const
{
	Ref< ShaderGraph > shaderGraph = gc_new< ShaderGraph >();
	std::stack< Node* > nodeStack;
	std::set< Node* > nodeVisited;
	RefArray< Edge > edges;

	// Get initial nodes.
	const RefArray< Node >& nodes = m_shaderGraph->getNodes();
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if (is_a< VertexOutput >(*i) || is_a< PixelOutput >(*i) || is_a< OutputPort >(*i))
			nodeStack.push(*i);
	}

	// Traverse graph, select combination branchs.
	while (!nodeStack.empty())
	{
		Ref< Node > node = nodeStack.top();
		nodeStack.pop();

		// Already visited this node?
		if (nodeVisited.find(node) != nodeVisited.end())
			continue;

		nodeVisited.insert(node);

		// Create combination of branch nodes.
		if (is_a< Branch >(node))
		{
			Ref< const InputPin > inputPin = node->findInputPin((combination & 1) ? L"True" : L"False");
			T_ASSERT (inputPin);

			Ref< const OutputPin > outputPin = node->findOutputPin(L"Output");
			T_ASSERT (outputPin);

			Ref< const OutputPin > sourceOutputPin = m_shaderGraphAdj->findSourcePin(inputPin);
			T_ASSERT (sourceOutputPin);

			RefArray< const InputPin > destinationInputPins;
			m_shaderGraphAdj->findDestinationPins(outputPin, destinationInputPins);
			T_ASSERT (!destinationInputPins.empty());

			for (RefArray< const InputPin >::iterator i = destinationInputPins.begin(); i != destinationInputPins.end(); ++i)
				edges.push_back(gc_new< Edge >(sourceOutputPin, *i));

			nodeStack.push(sourceOutputPin->getNode());
			combination >>= 1;
		}
		else
		{
			shaderGraph->addNode(node);

			int inputPinCount = node->getInputPinCount();
			for (int i = 0; i < inputPinCount; ++i)
			{
				Ref< const InputPin > inputPin = node->getInputPin(i);
				T_ASSERT (inputPin);

				Ref< Edge > edge = m_shaderGraphAdj->findEdge(inputPin);
				if (edge)
				{
					T_ASSERT (edge->getDestination() == inputPin);

					Ref< Node > sourceNode = edge->getSource()->getNode();
					if (!is_a< Branch >(sourceNode))
						edges.push_back(edge);

					nodeStack.push(sourceNode);
				}
			}
		}
	}

	// Add edges; add only edges which connect visited nodes.
	for (RefArray< Edge >::iterator i = edges.begin(); i != edges.end(); ++i)
	{
		bool haveSource = nodeVisited.find((*i)->getSource()->getNode()) != nodeVisited.end();
		bool haveDestination = nodeVisited.find((*i)->getDestination()->getNode()) != nodeVisited.end();
		if (haveSource && haveDestination)
			shaderGraph->addEdge(*i);
	}

	// Return a clone of combination graph as we don't want references between
	// graph instances.
	return DeepClone(shaderGraph).create< ShaderGraph >();
}

	}
}
