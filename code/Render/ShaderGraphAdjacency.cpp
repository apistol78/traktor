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

Edge* ShaderGraphAdjacency::findEdge(const InputPin* inputPin) const
{
	std::map< const InputPin*, Ref< Edge > >::const_iterator i = m_inputPinEdge.find(inputPin);
	return i != m_inputPinEdge.end() ? i->second : 0;
}

size_t ShaderGraphAdjacency::findEdges(const OutputPin* outputPin, RefArray< Edge >& outEdges) const
{
	std::map< const OutputPin*, RefArray< Edge > >::const_iterator i = m_outputPinEdges.find(outputPin);
	if (i != m_outputPinEdges.end())
		outEdges = i->second;
	else
		outEdges.resize(0);
	return outEdges.size();
}

const OutputPin* ShaderGraphAdjacency::findSourcePin(const InputPin* inputPin) const
{
	Edge* edge = findEdge(inputPin);
	return edge ? edge->getSource() : 0;
}

size_t ShaderGraphAdjacency::findDestinationPins(const OutputPin* outputPin, RefArray< const InputPin >& outDestinations) const
{
	RefArray< Edge > edges;
	findEdges(outputPin, edges);

	outDestinations.resize(edges.size());
	for (size_t i = 0; i < edges.size(); ++i)
		outDestinations[i] = edges[i]->getDestination();

	return outDestinations.size();
}

	}
}
