#include <algorithm>
#include "Render/ShaderGraphAdjacency.h"
#include "Render/ShaderGraph.h"
#include "Render/Node.h"
#include "Render/Edge.h"
#include "Render/OutputPin.h"
#include "Render/InputPin.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphAdjacency", ShaderGraphAdjacency, Object)

ShaderGraphAdjacency::ShaderGraphAdjacency(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
	const RefArray< Edge >& edges = m_shaderGraph->getEdges();
	for (RefArray< Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
	{
		m_inputPinEdge[(*i)->getDestination()] = *i;
		m_outputPinEdges[(*i)->getSource()].push_back(*i);
	}
}

Ref< Edge > ShaderGraphAdjacency::findEdge(const InputPin* inputPin) const
{
	std::map< const InputPin*, Ref< Edge > >::const_iterator i = m_inputPinEdge.find(inputPin);
	return i != m_inputPinEdge.end() ? i->second : 0;
}

uint32_t ShaderGraphAdjacency::findEdges(const OutputPin* outputPin, RefArray< Edge >& outEdges) const
{
	std::map< const OutputPin*, RefArray< Edge > >::const_iterator i = m_outputPinEdges.find(outputPin);
	if (i != m_outputPinEdges.end())
		outEdges = i->second;
	else
		outEdges.resize(0);
	return uint32_t(outEdges.size());
}

const OutputPin* ShaderGraphAdjacency::findSourcePin(const InputPin* inputPin) const
{
	Ref< Edge > edge = findEdge(inputPin);
	return edge ? edge->getSource() : 0;
}

uint32_t ShaderGraphAdjacency::findDestinationPins(const OutputPin* outputPin, std::vector< const InputPin* >& outDestinations) const
{
	RefArray< Edge > edges;
	findEdges(outputPin, edges);

	outDestinations.resize(edges.size());
	for (size_t i = 0; i < edges.size(); ++i)
		outDestinations[i] = edges[i]->getDestination();

	return uint32_t(outDestinations.size());
}

uint32_t ShaderGraphAdjacency::getDestinationCount(const OutputPin* outputPin) const
{
	std::map< const OutputPin*, RefArray< Edge > >::const_iterator i = m_outputPinEdges.find(outputPin);
	return i != m_outputPinEdges.end() ? uint32_t(i->second.size()) : 0;
}

	}
}
