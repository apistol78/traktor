/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Shader/Edge.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ShaderGraph", 0, ShaderGraph, ISerializable)

ShaderGraph::ShaderGraph()
{
	m_inputPinEdge.reserve(128);
	m_outputPinEdges.reserve(128);
}

ShaderGraph::ShaderGraph(const RefArray< Node >& nodes, const RefArray< Edge >& edges)
:	m_nodes(nodes)
,	m_edges(edges)
{
	m_inputPinEdge.reserve(m_edges.size());
	m_outputPinEdges.reserve(m_edges.size());

	updateAdjacency();
}

ShaderGraph::~ShaderGraph()
{
	removeAll();
}

void ShaderGraph::addNode(Node* node)
{
	T_ASSERT (std::find(m_nodes.begin(), m_nodes.end(), node) == m_nodes.end());
	m_nodes.push_back(node);
}

void ShaderGraph::removeNode(Node* node)
{
	RefArray< Node >::iterator i = std::find(m_nodes.begin(), m_nodes.end(), node);
	T_ASSERT (i != m_nodes.end());
	m_nodes.erase(i);
}

void ShaderGraph::addEdge(Edge* edge)
{
	T_ASSERT (std::find(m_edges.begin(), m_edges.end(), edge) == m_edges.end());
	
	m_edges.push_back(edge);

	m_inputPinEdge[edge->getDestination()] = edge;

	RefSet< Edge >*& set = m_outputPinEdges[edge->getSource()];
	if (!set)
		set = new RefSet< Edge >();

	set->insert(edge);
}

void ShaderGraph::removeEdge(Edge* edge)
{
	RefArray< Edge >::iterator i = std::find(m_edges.begin(), m_edges.end(), edge);
	T_ASSERT (i != m_edges.end());

	SmallMap< const InputPin*, Edge* >::iterator i1 = m_inputPinEdge.find(edge->getDestination());
	T_ASSERT (i1 != m_inputPinEdge.end());
	m_inputPinEdge.erase(i1);

	SmallMap< const OutputPin*, RefSet< Edge >* >::iterator i2 = m_outputPinEdges.find(edge->getSource());
	T_ASSERT (i2 != m_outputPinEdges.end());
	i2->second->erase(edge);

	m_edges.erase(i);
}

void ShaderGraph::removeAll()
{
	for (SmallMap< const OutputPin*, RefSet< Edge >* >::iterator i = m_outputPinEdges.begin(); i != m_outputPinEdges.end(); ++i)
		delete i->second;

	m_edges.resize(0);
	m_nodes.resize(0);

	m_inputPinEdge.clear();
	m_outputPinEdges.clear();
}

size_t ShaderGraph::findNodesOf(const TypeInfo& nodeType, RefArray< Node >& outNodes) const
{
	for (RefArray< Node >::const_iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
	{
		if (is_type_of(nodeType, type_of(*i)))
			outNodes.push_back(*i);
	}
	return outNodes.size();
}

Edge* ShaderGraph::findEdge(const InputPin* inputPin) const
{
	SmallMap< const InputPin*, Edge* >::const_iterator i = m_inputPinEdge.find(inputPin);
	return i != m_inputPinEdge.end() ? i->second : 0;
}

uint32_t ShaderGraph::findEdges(const OutputPin* outputPin, RefSet< Edge >& outEdges) const
{
	SmallMap< const OutputPin*, RefSet< Edge >* >::const_iterator i = m_outputPinEdges.find(outputPin);
	if (i != m_outputPinEdges.end())
		outEdges = *(i->second);
	else
		outEdges.clear();
	return uint32_t(outEdges.size());
}

const OutputPin* ShaderGraph::findSourcePin(const InputPin* inputPin) const
{
	Edge* edge = findEdge(inputPin);
	return edge ? edge->getSource() : 0;
}

uint32_t ShaderGraph::findDestinationPins(const OutputPin* outputPin, std::vector< const InputPin* >& outDestinations) const
{
	RefSet< Edge > edges;
	findEdges(outputPin, edges);

	outDestinations.resize(0);
	outDestinations.reserve(edges.size());
	for (RefSet< Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
		outDestinations.push_back((*i)->getDestination());

	return uint32_t(outDestinations.size());
}

uint32_t ShaderGraph::getDestinationCount(const OutputPin* outputPin) const
{
	SmallMap< const OutputPin*, RefSet< Edge >* >::const_iterator i = m_outputPinEdges.find(outputPin);
	return i != m_outputPinEdges.end() ? uint32_t(i->second->size()) : 0;
}

void ShaderGraph::detach(const Node* node)
{
	int32_t inputPinCount = node->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		Edge* edge = findEdge(node->getInputPin(i));
		if (edge)
			removeEdge(edge);
	}
	int32_t outputPinCount = node->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		RefSet< Edge > edges;
		findEdges(node->getOutputPin(i), edges);
		for (RefSet< Edge >::const_iterator j = edges.begin(); j != edges.end(); ++j)
			removeEdge(*j);
	}
	updateAdjacency();
}

void ShaderGraph::rewire(const OutputPin* outputPin, const OutputPin* newOutputPin)
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

void ShaderGraph::serialize(ISerializer& s)
{
	s >> MemberRefArray< Node >(L"nodes", m_nodes);
	s >> MemberRefArray< Edge >(L"edges", m_edges);

	if (s.getDirection() == ISerializer::SdRead)
		updateAdjacency();
}

void ShaderGraph::updateAdjacency()
{
	m_inputPinEdge.clear();

	// Clear all sets; keep associations so we don't have to reallocate for each update.
	for (SmallMap< const OutputPin*, RefSet< Edge >* >::iterator i = m_outputPinEdges.begin(); i != m_outputPinEdges.end(); ++i)
	{
		T_ASSERT (i->second);
		i->second->clear();
	}

	// Map edges input and output pins.
	for (RefArray< Edge >::iterator i = m_edges.begin(); i != m_edges.end(); ++i)
	{
		m_inputPinEdge[(*i)->getDestination()] = *i;

		RefSet< Edge >*& set = m_outputPinEdges[(*i)->getSource()];
		if (!set)
			set = new RefSet< Edge >();

		set->insert(*i);
	}
}

	}
}
