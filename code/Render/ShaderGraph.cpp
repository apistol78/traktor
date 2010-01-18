#include <algorithm>
#include "Render/ShaderGraph.h"
#include "Render/Node.h"
#include "Render/Edge.h"
#include "Render/OutputPin.h"
#include "Render/InputPin.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ShaderGraph", 0, ShaderGraph, ISerializable)

ShaderGraph::ShaderGraph()
{
}

ShaderGraph::ShaderGraph(const RefArray< Node >& nodes, const RefArray< Edge >& edges)
:	m_nodes(nodes)
,	m_edges(edges)
{
	updateAdjacency();
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
	m_outputPinEdges[edge->getSource()].insert(edge);
}

void ShaderGraph::removeEdge(Edge* edge)
{
	RefArray< Edge >::iterator i = std::find(m_edges.begin(), m_edges.end(), edge);
	if (i == m_edges.end())
		return;

	m_inputPinEdge.erase(edge->getDestination());

	std::map< const OutputPin*, RefSet< Edge > >::iterator i2 = m_outputPinEdges.find(edge->getSource());
	if (i2 != m_outputPinEdges.end())
		i2->second.erase(edge);

	m_edges.erase(i);
}

void ShaderGraph::removeAll()
{
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

Ref< Edge > ShaderGraph::findEdge(const InputPin* inputPin) const
{
	std::map< const InputPin*, Ref< Edge > >::const_iterator i = m_inputPinEdge.find(inputPin);
	return i != m_inputPinEdge.end() ? i->second : 0;
}

uint32_t ShaderGraph::findEdges(const OutputPin* outputPin, RefSet< Edge >& outEdges) const
{
	std::map< const OutputPin*, RefSet< Edge > >::const_iterator i = m_outputPinEdges.find(outputPin);
	if (i != m_outputPinEdges.end())
		outEdges = i->second;
	else
		outEdges.clear();
	return uint32_t(outEdges.size());
}

const OutputPin* ShaderGraph::findSourcePin(const InputPin* inputPin) const
{
	Ref< Edge > edge = findEdge(inputPin);
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
	std::map< const OutputPin*, RefSet< Edge > >::const_iterator i = m_outputPinEdges.find(outputPin);
	return i != m_outputPinEdges.end() ? uint32_t(i->second.size()) : 0;
}

bool ShaderGraph::serialize(ISerializer& s)
{
	s >> MemberRefArray< Node >(L"nodes", m_nodes);
	s >> MemberRefArray< Edge >(L"edges", m_edges);

	if (s.getDirection() == ISerializer::SdRead)
		updateAdjacency();

	return true;
}

void ShaderGraph::updateAdjacency()
{
	m_inputPinEdge.clear();
	m_outputPinEdges.clear();

	for (RefArray< Edge >::iterator i = m_edges.begin(); i != m_edges.end(); ++i)
	{
		Edge* edge = *i;
		m_inputPinEdge[edge->getDestination()] = edge;
		m_outputPinEdges[edge->getSource()].insert(edge);
	}
}

	}
}
