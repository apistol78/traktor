#include <algorithm>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Editor/Shader/Edge.h"
#include "Render/Editor/Shader/InputPin.h"
#include "Render/Editor/Shader/Node.h"
#include "Render/Editor/Shader/OutputPin.h"
#include "Render/Editor/Shader/ShaderGraph.h"

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
	updateInputPinToEdge();
	updateOutputPinDestinationCount();
}

ShaderGraph::~ShaderGraph()
{
	removeAll();
}

void ShaderGraph::addNode(Node* node)
{
	m_nodes.push_back(node);
}

void ShaderGraph::removeNode(Node* node)
{
	m_nodes.remove(node);
}

void ShaderGraph::addEdge(Edge* edge)
{
	T_ASSERT(std::find(m_edges.begin(), m_edges.end(), edge) == m_edges.end());
	m_edges.push_back(edge);

	T_ASSERT(m_inputPinToEdge.find(edge->getDestination()) == m_inputPinToEdge.end());
	m_inputPinToEdge[edge->getDestination()] = edge;
	m_outputPinDestinationCount[edge->getSource()]++;
}

void ShaderGraph::removeEdge(Edge* edge)
{
	if (m_edges.remove(edge))
	{
		T_ASSERT(m_inputPinToEdge.find(edge->getDestination()) != m_inputPinToEdge.end());
		m_inputPinToEdge.remove(edge->getDestination());
		m_outputPinDestinationCount[edge->getSource()]--;
	}
}

void ShaderGraph::removeAll()
{
	m_edges.resize(0);
	m_nodes.resize(0);
	m_inputPinToEdge.clear();
	m_outputPinDestinationCount.clear();
}

size_t ShaderGraph::findNodesOf(const TypeInfo& nodeType, RefArray< Node >& outNodes) const
{
	for (auto node : m_nodes)
	{
		if (is_type_of(nodeType, type_of(node)))
			outNodes.push_back(node);
	}
	return outNodes.size();
}

Edge* ShaderGraph::findEdge(const InputPin* inputPin) const
{
	auto it = m_inputPinToEdge.find(inputPin);
	return it != m_inputPinToEdge.end() ? it->second : nullptr;
}

uint32_t ShaderGraph::findEdges(const OutputPin* outputPin, RefSet< Edge >& outEdges) const
{
	outEdges.clear();
	for (auto edge : m_edges)
	{
		if (edge->getSource() == outputPin)
			outEdges.insert(edge);
	}
	return uint32_t(outEdges.size());
}

const OutputPin* ShaderGraph::findSourcePin(const InputPin* inputPin) const
{
	Edge* edge = findEdge(inputPin);
	return edge ? edge->getSource() : nullptr;
}

uint32_t ShaderGraph::findDestinationPins(const OutputPin* outputPin, AlignedVector< const InputPin* >& outDestinations) const
{
	outDestinations.resize(0);
	for (auto edge : m_edges)
	{
		if (edge->getSource() == outputPin)
			outDestinations.push_back(edge->getDestination());
	}
	return uint32_t(outDestinations.size());
}

uint32_t ShaderGraph::getDestinationCount(const OutputPin* outputPin) const
{
	auto it = m_outputPinDestinationCount.find(outputPin);
	return it != m_outputPinDestinationCount.end() ? it->second : 0;
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
		for (auto edge : edges)
			removeEdge(edge);
	}
}

void ShaderGraph::rewire(const OutputPin* outputPin, const OutputPin* newOutputPin)
{
	RefSet< Edge > outputEdges;
	findEdges(outputPin, outputEdges);
	if (newOutputPin)
	{
		for (auto edge : outputEdges)
			edge->setSource(newOutputPin);
	}
	else
	{
		for (auto edge : outputEdges)
			removeEdge(edge);
	}
}

void ShaderGraph::serialize(ISerializer& s)
{
	s >> MemberRefArray< Node >(L"nodes", m_nodes);
	s >> MemberRefArray< Edge >(L"edges", m_edges);

	if (s.getDirection() == ISerializer::SdRead)
	{
		updateInputPinToEdge();
		updateOutputPinDestinationCount();
	}
}

void ShaderGraph::updateInputPinToEdge()
{
	m_inputPinToEdge.clear();
	for (auto edge : m_edges)
		m_inputPinToEdge[edge->getDestination()] = edge;
}

void ShaderGraph::updateOutputPinDestinationCount()
{
	m_outputPinDestinationCount.clear();
	for (auto edge : m_edges)
		m_outputPinDestinationCount[edge->getSource()]++;
}

	}
}
