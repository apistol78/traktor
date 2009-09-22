#include <algorithm>
#include <stack>
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphOrderEvaluator.h"
#include "Render/ShaderGraph.h"
#include "Render/ShaderGraphAdjacency.h"
#include "Render/Nodes.h"
#include "Render/Edge.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

uint32_t calculateNodeHash(Node* node)
{
	std::pair< int, int > position = node->getPosition();
	std::wstring comment = node->getComment();

	node->setPosition(std::make_pair(0, 0));
	node->setComment(L"");

	uint32_t hash = DeepHash(node).get();

	node->setPosition(position);
	node->setComment(comment);

	return hash;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphOptimizer", ShaderGraphOptimizer, Object)

ShaderGraphOptimizer::ShaderGraphOptimizer(const ShaderGraph* shaderGraph)
:	m_insertedCount(0)
{
	m_shaderGraph = DeepClone(shaderGraph).create< ShaderGraph >();
	T_ASSERT (m_shaderGraph);

	m_shaderGraphAdj = gc_new< ShaderGraphAdjacency >(m_shaderGraph);
}

ShaderGraph* ShaderGraphOptimizer::removeUnusedBranches()
{
	std::stack< Node* > nodeStack;

	const RefArray< Node >& nodes = m_shaderGraph->getNodes();
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if (is_a< VertexOutput >(*i) || is_a< PixelOutput >(*i))
			nodeStack.push(*i);
	}

	RefArray< Node > usedNodes;
	RefArray< Edge > usedEdges;

	while (!nodeStack.empty())
	{
		Ref< Node > node = nodeStack.top();
		nodeStack.pop();

		if (std::find(usedNodes.begin(), usedNodes.end(), node) != usedNodes.end())
			continue;

		usedNodes.push_back(node);

		int inputPinCount = node->getInputPinCount();
		for (int i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			Ref< Edge > edge = m_shaderGraphAdj->findEdge(inputPin);
			if (edge)
			{
				const OutputPin* outputPin = edge->getSource();
				if (outputPin)
				{
					usedEdges.push_back(edge);
					nodeStack.push(outputPin->getNode());
				}
			}
		}
	}

	m_shaderGraph->removeAll();
	for (RefArray< Node >::iterator i = usedNodes.begin(); i != usedNodes.end(); ++i)
		m_shaderGraph->addNode(*i);
	for (RefArray< Edge >::iterator i = usedEdges.begin(); i != usedEdges.end(); ++i)
		m_shaderGraph->addEdge(*i);

	return m_shaderGraph;
}

ShaderGraph* ShaderGraphOptimizer::mergeBranches()
{
	uint32_t mergedNodes = 0;

	// Keep reference to array as we assume it will shrink
	// when we remove nodes.
	const RefArray< Node >& nodes = m_shaderGraph->getNodes();
	if (nodes.empty())
		return m_shaderGraph;

	// Calculate node hashes.
	std::map< const Node*, uint32_t > hash;
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		hash[*i] = calculateNodeHash(*i);

	// Merge single output nodes.
	for (uint32_t i = 0; i < uint32_t(nodes.size() - 1); ++i)
	{
		if (nodes[i]->getInputPinCount() != 0 || nodes[i]->getOutputPinCount() != 1)
			continue;

		for (uint32_t j = i + 1; j < uint32_t(nodes.size()); )
		{
			if (nodes[j]->getInputPinCount() != 0 || nodes[j]->getOutputPinCount() != 1)
			{
				++j;
				continue;
			}

			if (&nodes[i]->getType() != &nodes[j]->getType() || hash[nodes[i]] != hash[nodes[j]])
			{
				++j;
				continue;
			}

			// Identical nodes found; rewire edges.
			RefArray< Edge > edges;
			if (m_shaderGraphAdj->findEdges(nodes[j]->getOutputPin(0), edges) > 0)
			{
				for (RefArray< Edge >::iterator k = edges.begin(); k != edges.end(); ++k)
				{
					m_shaderGraph->removeEdge(*k);
					m_shaderGraph->addEdge(gc_new< Edge >(
						nodes[i]->getOutputPin(0),
						(*k)->getDestination()
					));
				}
				m_shaderGraphAdj = gc_new< ShaderGraphAdjacency >(m_shaderGraph);
			}

			// Remove redundant node.
			T_ASSERT (nodes[j]->getInputPinCount() == 0);
			T_ASSERT (nodes[j]->getOutputPinCount() == 1);
			m_shaderGraph->removeNode(nodes[j]);
			mergedNodes++;
		}
	}

	// Merge nodes which have same input signature; ie. are completely
	// connected to same nodes.
	for (;;)
	{
		bool merged = false;

		// Merge nodes; build array of nodes to remove.
		for (uint32_t i = 0; i < uint32_t(nodes.size() - 1); ++i)
		{
			for (uint32_t j = i + 1; j < uint32_t(nodes.size()); )
			{
				if (&nodes[i]->getType() != &nodes[j]->getType() || hash[nodes[i]] != hash[nodes[j]])
				{
					++j;
					continue;
				}

				int inputPinCount = nodes[i]->getInputPinCount();
				T_ASSERT (inputPinCount == nodes[j]->getInputPinCount());

				int outputPinCount = nodes[i]->getOutputPinCount();
				T_ASSERT (outputPinCount == nodes[j]->getOutputPinCount());

				// Are both nodes wired to same input nodes?
				bool wiredIdentical = true;
				for (int k = 0; k < inputPinCount; ++k)
				{
					const OutputPin* sourcePin1 = m_shaderGraphAdj->findSourcePin(nodes[i]->getInputPin(k));
					const OutputPin* sourcePin2 = m_shaderGraphAdj->findSourcePin(nodes[j]->getInputPin(k));
					if (sourcePin1 != sourcePin2)
					{
						wiredIdentical = false;
						break;
					}
				}
				if (!wiredIdentical)
				{
					++j;
					continue;
				}

				// Identically wired nodes found; rewire output edges.
				for (int k = 0; k < outputPinCount; ++k)
				{
					RefArray< Edge > edges;
					m_shaderGraphAdj->findEdges(nodes[j]->getOutputPin(k), edges);
					for (RefArray< Edge >::iterator m = edges.begin(); m != edges.end(); ++m)
					{
						m_shaderGraph->removeEdge(*m);
						m_shaderGraph->addEdge(gc_new< Edge >(
							nodes[i]->getOutputPin(k),
							(*m)->getDestination()
						));
					}
				}

				// Remove input edges.
				for (int k = 0; k < inputPinCount; ++k)
				{
					Ref< Edge > edge = m_shaderGraphAdj->findEdge(nodes[j]->getInputPin(k));
					if (edge)
						m_shaderGraph->removeEdge(edge);
				}

				// Update adjacency.
				if (outputPinCount > 0 || inputPinCount > 0)
					m_shaderGraphAdj = gc_new< ShaderGraphAdjacency >(m_shaderGraph);

				// Remove node; should be completely disconnected now.
				m_shaderGraph->removeNode(nodes[j]);

				merged = true;
				mergedNodes++;
			}
		}

		if (!merged)
			break;
	}

	log::debug << L"Merged " << mergedNodes << L" node(s)" << Endl;
	return m_shaderGraph;
}

ShaderGraph* ShaderGraphOptimizer::insertInterpolators()
{
	m_insertedCount = 0;
	updateOrderComplexity();

	RefArray< PixelOutput > pixelOutputNodes;
	m_shaderGraph->findNodesOf< PixelOutput >(pixelOutputNodes);

	for (RefArray< PixelOutput >::iterator i = pixelOutputNodes.begin(); i != pixelOutputNodes.end(); ++i)
		insertInterpolators(*i);
	
	log::debug << L"Inserted " << m_insertedCount << L" interpolator(s)" << Endl;
	return m_shaderGraph;
}

void ShaderGraphOptimizer::insertInterpolators(Node* node)
{
	// If we've reached an (manually placed) interpolator
	// then stop even if order to high.
	if (is_a< Interpolator >(node))
		return;

	// If we've already visited source node there is no
	// place to put an interpolator as we're in a loop.
	if (m_visited.find(node) != m_visited.end())
		return;

	m_visited.insert(node);

	for (int i = 0; i < node->getInputPinCount(); ++i)
	{
		const InputPin* inputPin = node->getInputPin(i);
		T_ASSERT (inputPin);

		const OutputPin* sourceOutputPin = m_shaderGraphAdj->findSourcePin(inputPin);
		if (!sourceOutputPin)
			continue;

		Ref< Node > sourceNode = sourceOutputPin->getNode();
		T_ASSERT (sourceNode);

		int inputOrder = m_orderComplexity[sourceNode];
		if (inputOrder <= ShaderGraphOrderEvaluator::OrLinear)
		{
			// We've reached low enough order; insert interpolator if linear and stop.
			if (inputOrder == ShaderGraphOrderEvaluator::OrLinear)
			{
				// Remove edge; replace with interpolator.
				Ref< Edge > edge = m_shaderGraphAdj->findEdge(inputPin);
				T_ASSERT (edge);

				m_shaderGraph->removeEdge(edge);
				edge = 0;

				// If this output pin already connected to an interpolator node then we reuse it.
				RefArray< Edge > outputEdges;
				m_shaderGraphAdj->findEdges(sourceOutputPin, outputEdges);
				for (RefArray< Edge >::iterator i = outputEdges.begin(); i != outputEdges.end(); ++i)
				{
					Ref< Node > targetNode = (*i)->getDestination()->getNode();
					if (is_a< Interpolator >(targetNode))
					{
						edge = gc_new< Edge >(targetNode->getOutputPin(0), inputPin);
						break;
					}
				}

				if (edge)
					m_shaderGraph->addEdge(edge);
				else
				{
					// Create new interpolator node.
					Ref< Interpolator > interpolator = gc_new< Interpolator >();
					std::pair< int, int > position = sourceNode->getPosition(); position.first += 64;
					interpolator->setPosition(position);
					m_shaderGraph->addNode(interpolator);

					// Create new edges.
					edge = gc_new< Edge >(sourceOutputPin, interpolator->getInputPin(0));
					m_shaderGraph->addEdge(edge);

					edge = gc_new< Edge >(interpolator->getOutputPin(0), inputPin);
					m_shaderGraph->addEdge(edge);

					m_insertedCount++;
				}

				m_shaderGraphAdj = gc_new< ShaderGraphAdjacency >(m_shaderGraph);
				updateOrderComplexity();
			}
		}
		else
		{
			// Input still have too high order; need to keep in pixel shader.
			insertInterpolators(sourceNode);
		}
	}
}

void ShaderGraphOptimizer::updateOrderComplexity()
{
	ShaderGraphOrderEvaluator evaluator(m_shaderGraph, m_shaderGraphAdj);
	const RefArray< Node >& nodes = m_shaderGraph->getNodes();
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		m_orderComplexity[*i] = evaluator.evaluate(*i);
}

	}
}
