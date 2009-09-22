#include <algorithm>
#include "Render/ShaderGraph.h"
#include "Render/Node.h"
#include "Render/Edge.h"
#include "Render/OutputPin.h"
#include "Render/InputPin.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

#if defined(_DEBUG)
#	define SANITY_CHECK() sanityCheck(m_nodes, m_edges)

void sanityCheck(const RefArray< Node >& nodes, const RefArray< Edge >& edges)
{
	for (RefArray< Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
	{
		const OutputPin* sourcePin = (*i)->getSource();
		T_ASSERT (sourcePin);

		const Node* sourceNode = sourcePin->getNode();
		T_ASSERT (std::find(nodes.begin(), nodes.end(), sourceNode) != nodes.end());

		const InputPin* destinationPin = (*i)->getDestination();
		T_ASSERT (destinationPin);

		const Node* destinationNode = destinationPin->getNode();
		T_ASSERT (std::find(nodes.begin(), nodes.end(), destinationNode) != nodes.end());
	}
}
#else
#	define SANITY_CHECK()
#endif

		}

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.render.ShaderGraph", ShaderGraph, Serializable)

ShaderGraph::ShaderGraph()
{
}

ShaderGraph::ShaderGraph(const RefArray< Node >& nodes, const RefArray< Edge >& edges)
:	m_nodes(nodes)
,	m_edges(edges)
{
	SANITY_CHECK();
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
}

void ShaderGraph::removeEdge(Edge* edge)
{
	RefArray< Edge >::iterator i = std::find(m_edges.begin(), m_edges.end(), edge);
	T_ASSERT (i != m_edges.end());
	m_edges.erase(i);
}

void ShaderGraph::removeAll()
{
	m_edges.resize(0);
	m_nodes.resize(0);
}

size_t ShaderGraph::findNodesOf(const Type& nodeType, RefArray< Node >& outNodes) const
{
	for (RefArray< Node >::const_iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
	{
		if (is_type_of(nodeType, (*i)->getType()))
			outNodes.push_back(*i);
	}
	return outNodes.size();
}

bool ShaderGraph::serialize(Serializer& s)
{
	SANITY_CHECK();
	s >> MemberRefArray< Node >(L"nodes", m_nodes);
	s >> MemberRefArray< Edge >(L"edges", m_edges);
	SANITY_CHECK();
	return true;
}

	}
}
