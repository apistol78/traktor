/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Graph.h"
#include "Render/Editor/Group.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Graph", 1, Graph, ISerializable)

Graph::Graph(const RefArray< Node >& nodes, const RefArray< Edge >& edges)
:	m_nodes(nodes)
,	m_edges(edges)
{
	updateInputPinToEdge();
	updateOutputPinDestinationCount();
}

Graph::~Graph()
{
	removeAll();
}

void Graph::addNode(Node* node)
{
	m_nodes.push_back(node);
}

void Graph::removeNode(Node* node)
{
	m_nodes.remove(node);
}

void Graph::addEdge(Edge* edge)
{
	T_ASSERT(std::find(m_edges.begin(), m_edges.end(), edge) == m_edges.end());
	m_edges.push_back(edge);

	T_ASSERT(m_inputPinToEdge.find(edge->getDestination()) == m_inputPinToEdge.end());
	m_inputPinToEdge[edge->getDestination()] = edge;
	m_outputPinDestinationCount[edge->getSource()]++;
}

void Graph::removeEdge(Edge* edge)
{
	T_ANONYMOUS_VAR(Ref< Edge >)(edge);
	if (m_edges.remove(edge))
	{
		T_ASSERT(m_inputPinToEdge.find(edge->getDestination()) != m_inputPinToEdge.end());
		m_inputPinToEdge.remove(edge->getDestination());
		m_outputPinDestinationCount[edge->getSource()]--;
	}
}

void Graph::addGroup(Group* group)
{
	m_groups.push_back(group);
}

void Graph::removeGroup(Group* group)
{
	m_groups.remove(group);
}

void Graph::removeAll()
{
	m_edges.resize(0);
	m_nodes.resize(0);
	m_groups.resize(0);
	m_inputPinToEdge.clear();
	m_outputPinDestinationCount.clear();
}

size_t Graph::findNodesOf(const TypeInfo& nodeType, RefArray< Node >& outNodes) const
{
	for (auto node : m_nodes)
	{
		if (is_type_of(nodeType, type_of(node)))
			outNodes.push_back(node);
	}
	return outNodes.size();
}

Edge* Graph::findEdge(const InputPin* inputPin) const
{
	auto it = m_inputPinToEdge.find(inputPin);
	return it != m_inputPinToEdge.end() ? it->second : nullptr;
}

RefArray< Edge > Graph::findEdges(const OutputPin* outputPin) const
{
	RefArray< Edge > edges;
	for (auto edge : m_edges)
	{
		if (edge->getSource() == outputPin)
			edges.push_back(edge);
	}
	return edges;
}

const OutputPin* Graph::findSourcePin(const InputPin* inputPin) const
{
	Edge* edge = findEdge(inputPin);
	return edge ? edge->getSource() : nullptr;
}

uint32_t Graph::findDestinationPins(const OutputPin* outputPin, AlignedVector< const InputPin* >& outDestinations) const
{
	outDestinations.resize(0);
	for (auto edge : m_edges)
	{
		if (edge->getSource() == outputPin)
			outDestinations.push_back(edge->getDestination());
	}
	return uint32_t(outDestinations.size());
}

uint32_t Graph::getDestinationCount(const OutputPin* outputPin) const
{
	auto it = m_outputPinDestinationCount.find(outputPin);
	return it != m_outputPinDestinationCount.end() ? it->second : 0;
}

void Graph::detach(const Node* node)
{
	const int32_t inputPinCount = node->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		Edge* edge = findEdge(node->getInputPin(i));
		if (edge)
			removeEdge(edge);
	}

	const int32_t outputPinCount = node->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		for (auto edge : findEdges(node->getOutputPin(i)))
			removeEdge(edge);
	}
}

void Graph::rewire(const OutputPin* outputPin, const OutputPin* newOutputPin)
{
	// Find all edges connected to output pin.
	RefArray< Edge > outputEdges = findEdges(outputPin);

	// Remove all those edges first.
	for (auto edge : outputEdges)
		removeEdge(edge);

	// Create new edges.
	if (newOutputPin)
	{
		for (auto edge : outputEdges)
			addEdge(new Edge(newOutputPin, edge->getDestination()));
	}
}

void Graph::replace(Node* oldNode, Node* newNode)
{
	// Ensure node isn't deleted until possibly the very end of this method.
	T_ANONYMOUS_VAR(Ref< Node >)(oldNode);

	// Replace node.
	m_nodes.remove(oldNode);
	m_nodes.push_back(newNode);

	// Rewire edges.
	RefArray< Edge > deleteEdges;
	RefArray< Edge > newEdges;
	for (auto edge : m_edges)
	{
		const OutputPin* newSource = nullptr;
		const InputPin* newDestination = nullptr;
		if (edge->getSource()->getNode() == oldNode)
		{
			newSource = newNode->findOutputPin(edge->getSource()->getName());
			deleteEdges.push_back(edge);
		}
		if (edge->getDestination()->getNode() == oldNode)
		{
			newDestination = newNode->findInputPin(edge->getDestination()->getName());
			deleteEdges.push_back(edge);
		}
		if (newSource || newDestination)
		{
			newEdges.push_back(new Edge(
				newSource != nullptr ? newSource : edge->getSource(),
				newDestination != nullptr ? newDestination : edge->getDestination()
			));
		}
	}
	for (auto edge : deleteEdges)
		m_edges.remove(edge);
	for (auto edge : newEdges)
		m_edges.push_back(edge);

	// Update mappings.
	updateInputPinToEdge();
	updateOutputPinDestinationCount();
}

void Graph::serialize(ISerializer& s)
{
	s >> MemberRefArray< Node >(L"nodes", m_nodes);
	s >> MemberRefArray< Edge >(L"edges", m_edges);

	if (s.getVersion< Graph >() >= 1)
		s >> MemberRefArray< Group >(L"groups", m_groups);

	if (s.getDirection() == ISerializer::Direction::Read)
	{
		updateInputPinToEdge();
		updateOutputPinDestinationCount();
	}
}

void Graph::updateInputPinToEdge()
{
	m_inputPinToEdge.clear();
	m_inputPinToEdge.reserve(m_edges.size());
	for (auto edge : m_edges)
		m_inputPinToEdge[edge->getDestination()] = edge;
}

void Graph::updateOutputPinDestinationCount()
{
	m_outputPinDestinationCount.clear();
	m_outputPinDestinationCount.reserve(m_edges.size());
	for (auto edge : m_edges)
		m_outputPinDestinationCount[edge->getSource()]++;
}

}
