/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <deque>
#include "Core/Log/Log.h"
#include "Core/Misc/ImmutableCheck.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/GraphTraverse.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphHash.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphOrderEvaluator.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphValidator.h"

namespace traktor::render
{
	namespace
	{

#if defined(_DEBUG)
#	define T_VALIDATE_SHADERGRAPH(sg) T_FATAL_ASSERT(ShaderGraphValidator(sg).validateIntegrity())
#else
#	define T_VALIDATE_SHADERGRAPH(sg)
#endif

struct CopyVisitor
{
	Ref< ShaderGraph > m_shaderGraph;

	bool operator () (Node* node)
	{
		m_shaderGraph->addNode(node);
		return true;
	}

	bool operator () (Edge* edge)
	{
		m_shaderGraph->addEdge(edge);
		return true;
	}
};

bool insideCycle(const ShaderGraph* shaderGraph, const OutputPin* outputPin)
{
	// No input pins on node means it cannot be part of a cycle.
	if (outputPin->getNode()->getInputPinCount() <= 0)
		return false;

	std::deque< const OutputPin* > scanOutputPins;
	SmallSet< const OutputPin* > scannedOutputPins;

	scanOutputPins.push_back(outputPin);
	scannedOutputPins.insert(outputPin);

	while (!scanOutputPins.empty())
	{
		const OutputPin* scanOutputPin = scanOutputPins.front();
		T_ASSERT(scanOutputPin);

		scanOutputPins.pop_front();

		const Node* node = scanOutputPin->getNode();
		T_ASSERT(node);

		const int32_t inputPinCount = node->getInputPinCount();
		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			T_ASSERT(inputPin);

			const OutputPin* sourceOutputPin = shaderGraph->findSourcePin(inputPin);
			if (sourceOutputPin)
			{
				if (sourceOutputPin == outputPin)
					return true;

				if (scannedOutputPins.find(sourceOutputPin) == scannedOutputPins.end())
				{
					scanOutputPins.push_back(sourceOutputPin);
					scannedOutputPins.insert(sourceOutputPin);
				}
			}
		}
	}

	return false;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphOptimizer", ShaderGraphOptimizer, Object)

ShaderGraphOptimizer::ShaderGraphOptimizer(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
}

Ref< ShaderGraph > ShaderGraphOptimizer::removeUnusedBranches(bool keepExternalAlways) const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	RefArray< Node > roots;
	for (auto node : m_shaderGraph->getNodes())
	{
		const INodeTraits* nodeTraits = INodeTraits::find(node);
		if (nodeTraits && nodeTraits->isRoot(m_shaderGraph, node))
			roots.push_back(node);
		else if (keepExternalAlways && is_a< External >(node))
			roots.push_back(node);
	}

	CopyVisitor visitor;
	visitor.m_shaderGraph = new ShaderGraph();
	GraphTraverse(m_shaderGraph, roots).preorder(visitor);

	T_VALIDATE_SHADERGRAPH(visitor.m_shaderGraph);
	return visitor.m_shaderGraph;
}

Ref< ShaderGraph > ShaderGraphOptimizer::mergeBranches() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	uint32_t mergedNodes = 0;

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	// Keep reference to array as we assume it will shrink
	// when we remove nodes.
	const RefArray< Node >& nodes = shaderGraph->getNodes();
	if (nodes.empty())
		return shaderGraph;

	// Calculate node hashes.
	SmallMap< const Node*, uint32_t > hash;
	hash.reserve(nodes.size());
	for (auto node : nodes)
		hash[node] = ShaderGraphHash(true, true).calculate(node);

	// Merge single output nodes.
	for (uint32_t i = 0; i < uint32_t(nodes.size() - 1); ++i)
	{
		if (nodes[i]->getInputPinCount() != 0 || nodes[i]->getOutputPinCount() != 1)
			continue;

		const TypeInfo* type0 = &type_of(nodes[i]);
		const uint32_t hash0 = hash[nodes[i]];

		for (uint32_t j = i + 1; j < uint32_t(nodes.size()); )
		{
			if (nodes[j]->getInputPinCount() != 0 || nodes[j]->getOutputPinCount() != 1)
			{
				++j;
				continue;
			}

			if (type0 != &type_of(nodes[j]) || hash0 != hash[nodes[j]])
			{
				++j;
				continue;
			}

			// Identical nodes found; rewire edges.
			RefArray< Edge > edges = shaderGraph->findEdges(nodes[j]->getOutputPin(0));
			if (!edges.empty())
			{
				for (auto edge : edges)
				{
					Ref< Edge > rewireEdge = new Edge(
						nodes[i]->getOutputPin(0),
						edge->getDestination()
					);

					shaderGraph->removeEdge(edge);
					shaderGraph->addEdge(rewireEdge);
				}
			}

			// Remove redundant node.
			T_ASSERT(nodes[j]->getInputPinCount() == 0);
			T_ASSERT(nodes[j]->getOutputPinCount() == 1);
			shaderGraph->removeNode(nodes[j]);
			mergedNodes++;
		}
	}

	// Merge nodes which have same input signature; ie. are completely
	// connected to same nodes.
	for (;;)
	{
		bool merged = false;

		// Merge nodes; build array of nodes to remove.
		for (uint32_t i = 0; i < (uint32_t)(nodes.size() - 1); ++i)
		{
			const TypeInfo* type0 = &type_of(nodes[i]);
			const uint32_t hash0 = hash[nodes[i]];
			const int32_t inputPinCount = nodes[i]->getInputPinCount();

			for (uint32_t j = i + 1; j < (uint32_t)(nodes.size()); )
			{
				if (type0 != &type_of(nodes[j]) || hash0 != hash[nodes[j]])
				{
					++j;
					continue;
				}

				T_ASSERT(inputPinCount == nodes[j]->getInputPinCount());

				const int32_t outputPinCount = nodes[i]->getOutputPinCount();
				T_ASSERT(outputPinCount == nodes[j]->getOutputPinCount());

				// Are both nodes wired to same input nodes?
				bool wiredIdentical = true;
				for (int32_t k = 0; k < inputPinCount; ++k)
				{
					const OutputPin* sourcePin1 = shaderGraph->findSourcePin(nodes[i]->getInputPin(k));
					const OutputPin* sourcePin2 = shaderGraph->findSourcePin(nodes[j]->getInputPin(k));
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
				for (int32_t k = 0; k < outputPinCount; ++k)
				{
					for (auto edge : shaderGraph->findEdges(nodes[j]->getOutputPin(k)))
					{
						Ref< Edge > rewireEdge = new Edge(
							nodes[i]->getOutputPin(k),
							edge->getDestination()
						);
						shaderGraph->removeEdge(edge);
						shaderGraph->addEdge(rewireEdge);
					}
				}

				// Remove input edges.
				for (int32_t k = 0; k < inputPinCount; ++k)
				{
					Edge* edge = shaderGraph->findEdge(nodes[j]->getInputPin(k));
					if (edge)
						shaderGraph->removeEdge(edge);
				}

				// Remove node; should be completely disconnected now.
				shaderGraph->removeNode(nodes[j]);

				merged = true;
				mergedNodes++;
			}
		}

		if (!merged)
			break;
	}

	T_DEBUG(L"Merged " << mergedNodes << L" node(s)");
	T_VALIDATE_SHADERGRAPH(shaderGraph);
	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphOptimizer::insertInterpolators() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	Ref< ShaderGraphOrderEvaluator > orderEvaluator = new ShaderGraphOrderEvaluator(shaderGraph);
	SmallSet< const Node* > visited;

	for (auto pixelOutputNode : shaderGraph->findNodesOf< PixelOutput >())
		insertInterpolators(visited, shaderGraph, pixelOutputNode, orderEvaluator);

	T_VALIDATE_SHADERGRAPH(shaderGraph);
	return shaderGraph;
}

void ShaderGraphOptimizer::insertInterpolators(
	SmallSet< const Node* >& visited,
	ShaderGraph* shaderGraph,
	Node* node,
	Ref< ShaderGraphOrderEvaluator >& inoutOrderEvaluator
) const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	// Should never reach vertex inputs.
	T_FATAL_ASSERT(!is_a< VertexInput >(node));

	// If we've reached an (manually placed) interpolator
	// then stop even if order to high.
	if (is_a< Interpolator >(node))
		return;

	// If we've already visited source node there is no
	// place to put an interpolator as we're in a loop.
	if (visited.find(node) != visited.end())
		return;
	visited.insert(node);

	for (int32_t i = 0; i < node->getInputPinCount(); ++i)
	{
		const InputPin* inputPin = node->getInputPin(i);
		T_ASSERT(inputPin);

		const OutputPin* sourceOutputPin = shaderGraph->findSourcePin(inputPin);
		if (!sourceOutputPin)
			continue;

		Ref< Node > sourceNode = sourceOutputPin->getNode();
		T_ASSERT(sourceNode);

		const bool isSwizzle = is_a< Swizzle >(sourceNode);
		const bool vertexMandatory = is_a< VertexInput >(sourceNode);
		const bool inCycle = insideCycle(shaderGraph, sourceOutputPin);

		PinOrder inputOrder = PinOrder::Constant;
		if (!vertexMandatory)
			inputOrder = inoutOrderEvaluator->evaluate(sourceOutputPin);

		if (vertexMandatory || (!isSwizzle && !inCycle && inputOrder <= PinOrder::Linear))
		{
			// We've reached low enough order; insert interpolator if linear and stop.
			if (vertexMandatory || inputOrder == PinOrder::Linear)
			{
				// Remove edge; replace with interpolator.
				Ref< Edge > edge = shaderGraph->findEdge(inputPin);
				T_ASSERT(edge);

				shaderGraph->removeEdge(edge);
				edge = nullptr;

				// If this output pin already connected to an interpolator node then we reuse it.
				for (auto outputEdge : shaderGraph->findEdges(sourceOutputPin))
				{
					Ref< Node > targetNode = outputEdge->getDestination()->getNode();
					if (is_a< Interpolator >(targetNode))
					{
						edge = new Edge(targetNode->getOutputPin(0), inputPin);
						break;
					}
				}

				if (edge)
					shaderGraph->addEdge(edge);
				else
				{
					// Create new interpolator node.
					Ref< Interpolator > interpolator = new Interpolator();
					std::pair< int, int > position = sourceNode->getPosition(); position.first += 64;
					interpolator->setPosition(position);
					shaderGraph->addNode(interpolator);

					// Create new edges.
					edge = new Edge(sourceOutputPin, interpolator->getInputPin(0));
					shaderGraph->addEdge(edge);

					edge = new Edge(interpolator->getOutputPin(0), inputPin);
					shaderGraph->addEdge(edge);
				}

				// Re-create order evaluator since we've modified the shader graph.
				inoutOrderEvaluator = new ShaderGraphOrderEvaluator(shaderGraph);
			}
		}
		else
		{
			// Input still have too high order; need to keep in pixel shader.
			insertInterpolators(visited, shaderGraph, sourceNode, inoutOrderEvaluator);
		}
	}
}

}
