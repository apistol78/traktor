/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Processor/Edge.h"
#include "Sound/Processor/Graph.h"
#include "Sound/Processor/Node.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.Graph", 0, Graph, ISerializable)

Graph::Graph()
{
	m_inputPinEdge.reserve(128);
	m_outputPinEdges.reserve(128);
}

Graph::Graph(const RefArray< Node >& nodes, const RefArray< Edge >& edges)
:	m_nodes(nodes)
,	m_edges(edges)
{
 	m_inputPinEdge.reserve(m_edges.size());
 	m_outputPinEdges.reserve(m_edges.size());

 	updateAdjacency();
}

Graph::~Graph()
{
	removeAll();
}

void Graph::addNode(Node* node)
{
	T_ASSERT(std::find(m_nodes.begin(), m_nodes.end(), node) == m_nodes.end());
	m_nodes.push_back(node);
}

void Graph::removeNode(Node* node)
{
	RefArray< Node >::iterator i = std::find(m_nodes.begin(), m_nodes.end(), node);
	T_ASSERT(i != m_nodes.end());
	m_nodes.erase(i);
}

void Graph::addEdge(Edge* edge)
{
	T_ASSERT(std::find(m_edges.begin(), m_edges.end(), edge) == m_edges.end());

	m_edges.push_back(edge);

	T_ASSERT(m_inputPinEdge[edge->getDestination()] == 0);
	m_inputPinEdge[edge->getDestination()] = edge;

	RefSet< Edge >*& set = m_outputPinEdges[edge->getSource()];
	if (!set)
		set = new RefSet< Edge >();

	set->insert(edge);
}

void Graph::removeEdge(Edge* edge)
{
	RefArray< Edge >::iterator i = std::find(m_edges.begin(), m_edges.end(), edge);
	T_ASSERT(i != m_edges.end());

	SmallMap< const InputPin*, Edge* >::iterator i1 = m_inputPinEdge.find(edge->getDestination());
	T_ASSERT(i1 != m_inputPinEdge.end());
	m_inputPinEdge.erase(i1);

	SmallMap< const OutputPin*, RefSet< Edge >* >::iterator i2 = m_outputPinEdges.find(edge->getSource());
	T_ASSERT(i2 != m_outputPinEdges.end());
	i2->second->erase(edge);

	m_edges.erase(i);
}

void Graph::removeAll()
{
	for (auto i = m_outputPinEdges.begin(); i != m_outputPinEdges.end(); ++i)
		delete i->second;

	m_edges.resize(0);
	m_nodes.resize(0);

	m_inputPinEdge.clear();
	m_outputPinEdges.clear();
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
	auto i = m_inputPinEdge.find(inputPin);
	return i != m_inputPinEdge.end() ? i->second : nullptr;
}

uint32_t Graph::findEdges(const OutputPin* outputPin, RefSet< Edge >& outEdges) const
{
	auto i = m_outputPinEdges.find(outputPin);
	if (i != m_outputPinEdges.end())
		outEdges = *(i->second);
	else
		outEdges.clear();
	return uint32_t(outEdges.size());
}

const OutputPin* Graph::findSourcePin(const InputPin* inputPin) const
{
	Edge* edge = findEdge(inputPin);
	return edge ? edge->getSource() : nullptr;
}

uint32_t Graph::findDestinationPins(const OutputPin* outputPin, AlignedVector< const InputPin* >& outDestinations) const
{
	RefSet< Edge > edges;
	findEdges(outputPin, edges);

	outDestinations.resize(0);
	outDestinations.reserve(edges.size());
	for (RefSet< Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
		outDestinations.push_back((*i)->getDestination());

	return uint32_t(outDestinations.size());
}

uint32_t Graph::getDestinationCount(const OutputPin* outputPin) const
{
	auto i = m_outputPinEdges.find(outputPin);
	return i != m_outputPinEdges.end() ? uint32_t(i->second->size()) : 0;
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
		RefSet< Edge > edges;
		findEdges(node->getOutputPin(i), edges);
		for (RefSet< Edge >::const_iterator j = edges.begin(); j != edges.end(); ++j)
			removeEdge(*j);
	}
	updateAdjacency();
}

void Graph::rewire(const OutputPin* outputPin, const OutputPin* newOutputPin)
{
	RefSet< Edge > outputEdges;
	findEdges(outputPin, outputEdges);

	if (newOutputPin)
	{
		for (RefSet< Edge >::const_iterator j = outputEdges.begin(); j != outputEdges.end(); ++j)
			(*j)->setSource(newOutputPin);
	}
	else
	{
		for (RefSet< Edge >::const_iterator j = outputEdges.begin(); j != outputEdges.end(); ++j)
			removeEdge(*j);
	}

	updateAdjacency();
}

void Graph::serialize(ISerializer& s)
{
	s >> MemberRefArray< Node >(L"nodes", m_nodes);
	s >> MemberRefArray< Edge >(L"edges", m_edges);

	if (s.getDirection() == ISerializer::Direction::Read)
		updateAdjacency();
}

void Graph::updateAdjacency()
{
	m_inputPinEdge.clear();

	// Clear all sets; keep associations so we don't have to reallocate for each update.
	for (auto i = m_outputPinEdges.begin(); i != m_outputPinEdges.end(); ++i)
	{
		T_ASSERT(i->second);
		i->second->clear();
	}

	// Map edges input and output pins.
	for (auto edge : m_edges)
	{
		m_inputPinEdge[edge->getDestination()] = edge;

		RefSet< Edge >*& set = m_outputPinEdges[edge->getSource()];
		if (!set)
			set = new RefSet< Edge >();

		set->insert(edge);
	}
}

}
