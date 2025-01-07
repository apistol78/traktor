/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Graph.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"

#include <functional>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! Graph traversal helper.
 * \ingroup Render
 */
class GraphTraverse
{
public:
	explicit GraphTraverse(const Graph* graph, Node* root)
		: m_graph(graph)
	{
		m_roots.push_back(root);
	}

	explicit GraphTraverse(const Graph* graph, const RefArray< Node >& roots)
		: m_graph(graph)
		, m_roots(roots)
	{
	}

	template < typename VisitorType >
	void preorder(VisitorType& visitor)
	{
		SmallSet< Node* > visited;
		for (auto root : m_roots)
			preorderImpl(root, visitor, visited);
	}

	template < typename VisitorType >
	void postorder(VisitorType& visitor)
	{
		SmallSet< Node* > visited;
		for (auto root : m_roots)
			postorderImpl(root, visitor, visited);
	}

	void preorder(const std::function< bool(const Node*) >& fn)
	{
		LambdaVisitor visitor(fn);
		preorder(visitor);
	}

	void postorder(const std::function< bool(const Node*) >& fn)
	{
		LambdaVisitor visitor(fn);
		postorder(visitor);
	}

	template < typename VisitorType >
	void breadthFirst(VisitorType& visitor)
	{
		SmallSet< Node* > visited;
		breadthFirstImpl(m_roots, visitor, visited);
	}

private:
	const Graph* m_graph;
	RefArray< Node > m_roots;

	class LambdaVisitor
	{
	public:
		LambdaVisitor(const std::function< bool(const Node*) >& fn)
			: m_fn(fn)
		{
		}

		bool operator()(const Node* node)
		{
			return m_fn(node);
		}

		bool operator()(const Edge* edge)
		{
			return true;
		}

	private:
		std::function< bool(const Node*) > m_fn;
	};

	template < typename VisitorType >
	void preorderImpl(Node* node, VisitorType& visitor, SmallSet< Node* >& visited)
	{
		if (visited.find(node) != visited.end())
			return;
		visited.insert(node);

		if (!visitor(node))
			return;

		const uint32_t inputPinCount = node->getInputPinCount();
		for (uint32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			T_ASSERT(inputPin);

			Ref< Edge > edge = m_graph->findEdge(inputPin);
			if (edge)
			{
				if (!visitor(edge))
					continue;

				if (edge->getSource() != nullptr && edge->getDestination() != nullptr)
					preorderImpl(edge->getSource()->getNode(), visitor, visited);
			}
		}
	}

	template < typename VisitorType >
	void postorderImpl(Node* node, VisitorType& visitor, SmallSet< Node* >& visited)
	{
		if (visited.find(node) != visited.end())
			return;
		visited.insert(node);

		const uint32_t inputPinCount = node->getInputPinCount();
		for (uint32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			T_ASSERT(inputPin);

			Ref< Edge > edge = m_graph->findEdge(inputPin);
			if (edge)
			{
				if (!visitor(edge))
					continue;

				if (edge->getSource() != nullptr && edge->getDestination() != nullptr)
					postorderImpl(edge->getSource()->getNode(), visitor, visited);
			}
		}

		if (!visitor(node))
			return;
	}

	template < typename VisitorType >
	void breadthFirstImpl(const RefArray< Node >& nodes, VisitorType& visitor, SmallSet< Node* >& visited)
	{
		RefArray< Node > children;
		for (auto node : nodes)
		{
			if (visited.find(node) != visited.end())
				continue;
			visited.insert(node);

			if (!visitor(node))
				continue;

			const uint32_t inputPinCount = node->getInputPinCount();
			for (uint32_t j = 0; j < inputPinCount; ++j)
			{
				const InputPin* inputPin = node->getInputPin(j);
				T_ASSERT(inputPin);

				Ref< Edge > edge = m_graph->findEdge(inputPin);
				if (edge)
				{
					if (!visitor(edge))
						continue;

					if (edge->getSource() != nullptr && edge->getDestination() != nullptr)
						children.push_back(edge->getSource()->getNode());
				}
			}
		}
		if (!children.empty())
			breadthFirstImpl(children, visitor, visited);
	}
};

/*! Traverse shader graph nodes through a root set.
 * \ingroup Render
 *
 * \param graph Shader graph.
 * \param roots Root nodes.
 * \param visitor Node and edge visitor.
 */
template < typename VisitorType >
void graphTraverse(const Graph* graph, const RefArray< Node >& roots, VisitorType& visitor)
{
	GraphTraverse(graph, roots).preorder(visitor);
}

/*! Check if an input propagate to a target node.
 * \ingroup Render
 */
bool T_DLLCLASS doesInputPropagateToNode(const Graph* graph, const InputPin* inputPin, Node* targetNode);

/*! Check if two pins are connected.
 * \ingroup Render
 */
bool T_DLLCLASS arePinsConnected(const Graph* graph, const OutputPin* outputPin, const InputPin* inputPin);

/*! Get merging, common, outputs from a set of input pins.
 * \ingroup Render
 */
void T_DLLCLASS getMergingOutputs(const Graph* graph, const AlignedVector< const InputPin* >& inputPins, AlignedVector< const OutputPin* >& outMergedOutputPins);

/*! Get non-dependent outputs from a an input and a set of dependent output pins.
 * \ingroup Render
 */
void T_DLLCLASS getNonDependentOutputs(const Graph* graph, const InputPin* inputPin, const AlignedVector< const OutputPin* >& dependentOutputPins, AlignedVector< const OutputPin* >& outOutputPins);

}
