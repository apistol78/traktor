#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include "Render/FragmentLinker.h"
#include "Render/ShaderGraph.h"
#include "Render/Node.h"
#include "Render/Nodes.h"
#include "Render/External.h"
#include "Render/Edge.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct OutputPortPredicate
{
	std::wstring m_name;

	OutputPortPredicate(const std::wstring& name)
	:	m_name(name)
	{
	}

	bool operator () (const OutputPort* outputPort) const
	{
		return outputPort->getName() == m_name;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.FragmentLinker", FragmentLinker, Object)

FragmentLinker::FragmentLinker()
:	m_fragmentReader(0)
{
}

FragmentLinker::FragmentLinker(FragmentReader& fragmentReader)
:	m_fragmentReader(&fragmentReader)
{
}

ShaderGraph* FragmentLinker::resolve(const ShaderGraph* shaderGraph)
{
	if (!shaderGraph)
		return 0;

	Ref< ShaderGraph > resolvedGraph = gc_new< ShaderGraph >();

	std::map< const Node*, Ref< const ShaderGraph > > fragmentGraphs;
	RefArray< Edge > unresolvedEdges;

	const RefArray< Edge >& edges = shaderGraph->getEdges();
	for (RefArray< Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
		unresolvedEdges.push_back(*i);

	const RefArray< Node >& nodes = shaderGraph->getNodes();
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		Node* node = *i;
		if (is_a< External >(node))
		{
			Guid fragmentGuid = static_cast< const External* >(node)->getFragmentGuid();

			Ref< const ShaderGraph > fragmentGraph = m_fragmentReader->read(fragmentGuid);
			if (!fragmentGraph)
			{
				log::error << L"Unable to read fragment " << fragmentGuid.format() << Endl;
				return 0;
			}

			// Create a clone of the graph as we don't want duplicate references in graphs.
			fragmentGraph = DeepClone(fragmentGraph).create< ShaderGraph >();
			T_ASSERT (fragmentGraph);

			// Resolve sub external references.
			fragmentGraph = resolve(fragmentGraph);
			if (!fragmentGraph)
			{
				log::error << L"Unable to resolve fragment " << fragmentGuid.format() << Endl;
				return 0;
			}

			const RefArray< Edge >& fragmentEdges = fragmentGraph->getEdges();
			for (RefArray< Edge >::const_iterator j = fragmentEdges.begin(); j != fragmentEdges.end(); ++j)
			{
				if (
					!is_a< InputPort >((*j)->getSource()->getNode()) && 
					!is_a< OutputPort >((*j)->getDestination()->getNode())
				)
					unresolvedEdges.push_back(*j);
			}

			const RefArray< Node >& fragmentNodes = fragmentGraph->getNodes();
			for (RefArray< Node >::const_iterator j = fragmentNodes.begin(); j != fragmentNodes.end(); ++j)
			{
				Node* fragmentNode = *j;
				T_ASSERT (!is_a< External >(fragmentNode));

				if (!is_a< InputPort >(fragmentNode) && !is_a< OutputPort >(fragmentNode))
					resolvedGraph->addNode(fragmentNode);
			}

			fragmentGraphs[node] = fragmentGraph;
		}
		else
			resolvedGraph->addNode(node);
	}

	for (RefArray< Edge >::iterator i = unresolvedEdges.begin(); i != unresolvedEdges.end(); ++i)
	{
		Edge* edge = *i;

		Ref< const OutputPin > sourcePin = edge->getSource();
		if (is_a< External >(sourcePin->getNode()))
		{
			Ref< const ShaderGraph > fragmentGraph = fragmentGraphs[sourcePin->getNode()];
			T_ASSERT (fragmentGraph);

			const RefArray< Node >& fragmentNodes = fragmentGraph->getNodes();
			for (RefArray< Node >::const_iterator j = fragmentNodes.begin(); j != fragmentNodes.end(); ++j)
			{
				OutputPort* fragmentPort = dynamic_type_cast< OutputPort* >(*j);
				if (fragmentPort && fragmentPort->getName() == sourcePin->getName())
				{
					sourcePin = fragmentGraph->findSourcePin(fragmentPort->findInputPin(L"Input"));
					T_ASSERT (sourcePin);
					break;
				}
			}
		}

		RefArray< const InputPin > destinationPins;
		Ref< const InputPin > destinationPin = edge->getDestination();
		if (is_a< External >(destinationPin->getNode()))
		{
			Ref< const ShaderGraph > fragmentGraph = fragmentGraphs[destinationPin->getNode()];
			T_ASSERT (fragmentGraph);

			const RefArray< Node >& fragmentNodes = fragmentGraph->getNodes();
			for (RefArray< Node >::const_iterator j = fragmentNodes.begin(); j != fragmentNodes.end(); ++j)
			{
				InputPort* fragmentPort = dynamic_type_cast< InputPort* >(*j);
				if (fragmentPort && fragmentPort->getName() == destinationPin->getName())
				{
					fragmentGraph->findDestinationPins(fragmentPort->findOutputPin(L"Output"), destinationPins);
					break;
				}
			}
		}
		else
			destinationPins.push_back(destinationPin);

		for (RefArray< const InputPin >::iterator j = destinationPins.begin(); j != destinationPins.end(); ++j)
		{
			T_ASSERT (!is_a< External >(sourcePin->getNode()));
			T_ASSERT (!is_a< External >((*j)->getNode()));
			resolvedGraph->addEdge(gc_new< Edge >(sourcePin, *j));
		}
	}

	return resolvedGraph;
}

ShaderGraph* FragmentLinker::merge(const ShaderGraph* shaderGraphLeft, const ShaderGraph* shaderGraphRight)
{
	if (!shaderGraphLeft || !shaderGraphRight)
		return 0;

	const RefArray< Node >& nodesLeft = shaderGraphLeft->getNodes();
	const RefArray< Node >& nodesRight = shaderGraphRight->getNodes();

	const RefArray< Edge >& edgesLeft = shaderGraphLeft->getEdges();
	const RefArray< Edge >& edgesRight = shaderGraphRight->getEdges();

	RefArray< Node > resolvedNodes; resolvedNodes.reserve(nodesLeft.size() + nodesRight.size());
	RefArray< Edge > resolvedEdges; resolvedEdges.reserve(edgesLeft.size() + edgesRight.size());

	std::set< OutputPort* > outputPorts;
	std::set< InputPort* > inputPorts;

	for (RefArray< Node >::const_iterator i = nodesLeft.begin(); i != nodesLeft.end(); ++i)
	{
		if (!is_a< OutputPort >(*i))
			resolvedNodes.push_back(*i);
		else
			outputPorts.insert(static_cast< OutputPort* >(*i));
	}

	for (RefArray< Node >::const_iterator i = nodesRight.begin(); i != nodesRight.end(); ++i)
	{
		if (!is_a< InputPort >(*i))
			resolvedNodes.push_back(*i);
		else
			inputPorts.insert(static_cast< InputPort* >(*i));
	}	

	for (RefArray< Edge >::const_iterator i = edgesLeft.begin(); i != edgesLeft.end(); ++i)
	{
		Node* sourceNode = (*i)->getSource()->getNode();
		Node* destinationNode = (*i)->getDestination()->getNode();

		bool foundSource = std::find(resolvedNodes.begin(), resolvedNodes.end(), sourceNode) != resolvedNodes.end();
		bool foundDestination = std::find(resolvedNodes.begin(), resolvedNodes.end(), destinationNode) != resolvedNodes.end();
		if (foundSource && foundDestination)
			resolvedEdges.push_back(*i);
	}

	for (RefArray< Edge >::const_iterator i = edgesRight.begin(); i != edgesRight.end(); ++i)
	{
		Node* sourceNode = (*i)->getSource()->getNode();
		Node* destinationNode = (*i)->getDestination()->getNode();

		bool foundSource = std::find(resolvedNodes.begin(), resolvedNodes.end(), sourceNode) != resolvedNodes.end();
		bool foundDestination = std::find(resolvedNodes.begin(), resolvedNodes.end(), destinationNode) != resolvedNodes.end();
		if (foundSource && foundDestination)
			resolvedEdges.push_back(*i);
	}

	for (RefArray< Node >::const_iterator i = nodesRight.begin(); i != nodesRight.end(); ++i)
	{
		InputPort* inputPort = dynamic_type_cast< InputPort* >(*i);
		if (!inputPort)
			continue;

		RefArray< const InputPin > destinationPins;
		if (!shaderGraphRight->findDestinationPins(inputPort->getOutputPin(0), destinationPins))
			continue;

		std::set< OutputPort* >::iterator j = std::find_if(outputPorts.begin(), outputPorts.end(), OutputPortPredicate(inputPort->getName()));
		if (j != outputPorts.end())
		{
			Ref< const OutputPin > sourcePin = shaderGraphLeft->findSourcePin((*j)->getInputPin(0));

			bool foundSource = std::find(resolvedNodes.begin(), resolvedNodes.end(), sourcePin->getNode()) != resolvedNodes.end();
			if (!foundSource)
				resolvedNodes.push_back(sourcePin->getNode());

			for (RefArray< const InputPin >::iterator k = destinationPins.begin(); k != destinationPins.end(); ++k)
			{
				bool foundDestination = std::find(resolvedNodes.begin(), resolvedNodes.end(),(*k)->getNode()) != resolvedNodes.end();
				if (!foundDestination)
					resolvedNodes.push_back((*k)->getNode());

				resolvedEdges.push_back(gc_new< Edge >(sourcePin, *k));
			}
		}
		else
		{
			bool foundSource = std::find(resolvedNodes.begin(), resolvedNodes.end(), inputPort) != resolvedNodes.end();
			if (!foundSource)
				resolvedNodes.push_back(inputPort);

			for (RefArray< const InputPin >::iterator k = destinationPins.begin(); k != destinationPins.end(); ++k)
			{
				bool foundDestination = std::find(resolvedNodes.begin(), resolvedNodes.end(),(*k)->getNode()) != resolvedNodes.end();
				if (!foundDestination)
					resolvedNodes.push_back((*k)->getNode());

				resolvedEdges.push_back(gc_new< Edge >(inputPort->getOutputPin(0), *k));
			}
		}
	}

	Ref< ShaderGraph > resolvedGraph = gc_new< ShaderGraph >(
		cref(resolvedNodes),
		cref(resolvedEdges)
	);

	return resolvedGraph;
}

	}
}
