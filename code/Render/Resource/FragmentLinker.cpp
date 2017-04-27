/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <map>
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/External.h"
#include "Render/Shader/Edge.h"
#include "Render/Resource/FragmentLinker.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_PortConnectionNode_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_PortConnectionNode_o[] = { { L"Output" }, { 0 } };

class PortConnectionNode : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	PortConnectionNode()
	:	ImmutableNode(c_PortConnectionNode_i, c_PortConnectionNode_o)
	{
	}
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.FragmentLinker.PortConnectionNode", 0, PortConnectionNode, ImmutableNode)

template < typename NodeType >
struct NodeTypePred
{
	bool operator () (Node* node) const
	{
		return &type_of(node) == &type_of< NodeType >();
	}
};

#if defined(_DEBUG)

void validateShaderGraph(const ShaderGraph* shaderGraph, bool noExternalAllowed)
{
	const RefArray< Node >& nodes = shaderGraph->getNodes();
	const RefArray< Edge >& edges = shaderGraph->getEdges();

	for (RefArray< Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
	{
		RefArray< Node >::const_iterator i0 = std::find(nodes.begin(), nodes.end(), (*i)->getSource()->getNode());
		T_ASSERT (i0 != nodes.end());

		RefArray< Node >::const_iterator i1 = std::find(nodes.begin(), nodes.end(), (*i)->getDestination()->getNode());
		T_ASSERT (i1 != nodes.end());
	}

	if (noExternalAllowed)
	{
		for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
			T_ASSERT (!is_a< External >(*i));
	}
}

#endif

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.FragmentLinker", FragmentLinker, Object)

FragmentLinker::FragmentLinker()
:	m_fragmentReader(0)
{
}

FragmentLinker::FragmentLinker(const IFragmentReader& fragmentReader)
:	m_fragmentReader(&fragmentReader)
{
}

Ref< ShaderGraph > FragmentLinker::resolve(const ShaderGraph* shaderGraph, bool fullResolve, const Guid* optionalShaderGraphGuid) const
{
	Ref< ShaderGraph > originalShaderGraph = DeepClone(shaderGraph).create< ShaderGraph >();
	RefArray< Edge > resolvedEdges = originalShaderGraph->getEdges();
	RefArray< Node > resolvedNodes;

	// Move over nodes; import external fragments when external nodes is encountered.
	const RefArray< Node >& originalNodes = originalShaderGraph->getNodes();
	for (RefArray< Node >::const_iterator i = originalNodes.begin(); i != originalNodes.end(); ++i)
	{
		const External* externalNode = dynamic_type_cast< const External* >(*i);
		if (externalNode)
		{
			// Read external fragment graph.
			Ref< const ShaderGraph > fragmentShaderGraph = m_fragmentReader->read(externalNode->getFragmentGuid());
			if (!fragmentShaderGraph)
			{
				if (optionalShaderGraphGuid)
					log::error << L"Fragment linkage of \"" << optionalShaderGraphGuid->format() << L"\" failed; unable to read fragment \"" << externalNode->getFragmentGuid().format() << L"\"" << Endl;
				else
					log::error << L"Fragment linkage failed; unable to read fragment \"" << externalNode->getFragmentGuid().format() << L"\"" << Endl;
				return 0;
			}

#if defined(_DEBUG)
			validateShaderGraph(fragmentShaderGraph, false);
#endif

			// Recursively resolve externals in fragment graph.
			if (fullResolve)
			{
				fragmentShaderGraph = resolve(fragmentShaderGraph, fullResolve, &externalNode->getFragmentGuid());
				if (!fragmentShaderGraph)
					return 0;

#if defined(_DEBUG)
				validateShaderGraph(fragmentShaderGraph, true);
#endif
			}

			std::map< std::wstring, Ref< Node > > inputEstablishedPorts;
			std::map< std::wstring, Ref< Node > > outputEstablishedPorts;

			RefArray< InputPort > fragmentInputPorts;
			fragmentShaderGraph->findNodesOf< InputPort >(fragmentInputPorts);

			for (RefArray< InputPort >::const_iterator j = fragmentInputPorts.begin(); j != fragmentInputPorts.end(); ++j)
			{
				std::wstring inputPortName = (*j)->getName();
				bool replaceWithValue = true;

				if ((*j)->isConnectable())
				{
					// Find input pin by input port.
					const InputPin* externalInputPin = externalNode->findInputPin(inputPortName);
					if (externalInputPin)
					{
						// Create "established port" node and move all edges from input port to established port.
						Ref< PortConnectionNode > inputPortConnectionNode = new PortConnectionNode();
						for (RefArray< Edge >::iterator k = resolvedEdges.begin(); k != resolvedEdges.end(); ++k)
						{
							if ((*k)->getDestination() == externalInputPin)
							{
								(*k)->setDestination(inputPortConnectionNode->getInputPin(0));
								replaceWithValue = false;
							}
						}
						if (!replaceWithValue)
						{
							T_ASSERT (inputEstablishedPorts[inputPortName] == 0);
							inputEstablishedPorts[inputPortName] = inputPortConnectionNode;
							resolvedNodes.push_back(inputPortConnectionNode);
						}
						else if (!(*j)->isOptional())
						{
							if (optionalShaderGraphGuid)
								log::error << L"Fragment linkage of \"" << optionalShaderGraphGuid->format() << L"\" non-optional pin \"" << inputPortName << L"\" unconnected." << Endl;
							else
								log::error << L"Fragment linkage failed;  non-optional pin \"" << inputPortName << L"\" unconnected." << Endl;
							return 0;
						}
					}
					else if (!(*j)->isOptional())
					{
						if (optionalShaderGraphGuid)
							log::error << L"Fragment linkage of \"" << optionalShaderGraphGuid->format() << L"\" failed; no such pin \"" << inputPortName << L"\"." << Endl;
						else
							log::error << L"Fragment linkage failed; no such pin \"" << inputPortName << L"\"." << Endl;
						return 0;
					}
				}

				// Non-connected or value-only input port; replace with scalar value node.
				if (replaceWithValue && (*j)->haveDefaultValue())
				{
					float value = externalNode->getValue(inputPortName, (*j)->getDefaultValue());
					Ref< Scalar > valueNode = new Scalar(value);
					T_ASSERT (inputEstablishedPorts[inputPortName] == 0);
					inputEstablishedPorts[inputPortName] = valueNode;
					resolvedNodes.push_back(valueNode);
				}
			}

			for (int j = 0; j < externalNode->getOutputPinCount(); ++j)
			{
				std::wstring outputPortName = externalNode->getOutputPin(j)->getName();

				Ref< PortConnectionNode > outputPortConnectionNode = new PortConnectionNode();
				T_ASSERT (outputEstablishedPorts[outputPortName] == 0);
				outputEstablishedPorts[outputPortName] = outputPortConnectionNode;
				resolvedNodes.push_back(outputPortConnectionNode);

				for (RefArray< Edge >::iterator k = resolvedEdges.begin(); k != resolvedEdges.end(); ++k)
				{
					if ((*k)->getSource() == externalNode->getOutputPin(j))
						(*k)->setSource(outputPortConnectionNode->getOutputPin(0));
				}
			}
			
			// Move over edges, if "established port" nodes encountered replace with proper pins.
			const RefArray< Edge >& fragmentEdges = fragmentShaderGraph->getEdges();
			for (RefArray< Edge >::const_iterator j = fragmentEdges.begin(); j != fragmentEdges.end(); ++j)
			{
				if (
					!(*j)->getSource() ||
					!(*j)->getDestination()
				)
					continue;

				const OutputPin* sourcePin;
				const InputPin* destinationPin;

				if (const InputPort* inputPort = dynamic_type_cast< const InputPort* >((*j)->getSource()->getNode()))
				{
					std::map< std::wstring, Ref< Node > >::iterator it = inputEstablishedPorts.find(inputPort->getName());
					sourcePin = (it != inputEstablishedPorts.end()) ? it->second->getOutputPin(0) : 0;
				}
				else
					sourcePin = (*j)->getSource();

				if (const OutputPort* outputPort = dynamic_type_cast< const OutputPort* >((*j)->getDestination()->getNode()))
				{
					std::map< std::wstring, Ref< Node > >::iterator it = outputEstablishedPorts.find(outputPort->getName());
					destinationPin = (it != outputEstablishedPorts.end()) ? it->second->getInputPin(0) : 0;
				}
				else
					destinationPin = (*j)->getDestination();

				if (sourcePin && destinationPin)
					resolvedEdges.push_back(new Edge(
						sourcePin,
						destinationPin
					));
			}

			// Move over all non-port nodes as-is.
			const RefArray< Node >& fragmentNodes = fragmentShaderGraph->getNodes();
			for (RefArray< Node >::const_iterator j = fragmentNodes.begin(); j != fragmentNodes.end(); ++j)
			{
				T_ASSERT (!fullResolve || !is_a< External >(*j));
				if (
					!is_a< InputPort >(*j) &&
					!is_a< OutputPort >(*j)
				)
					resolvedNodes.push_back(*j);
			}
		}
		else
		{
			// Move over non-external nodes as-is.
			resolvedNodes.push_back(*i);
		}
	}

	// Re-wire edges which has been temporarily connected to a "port connection" node.
	RefArray< Node >::iterator i = std::find_if(resolvedNodes.begin(), resolvedNodes.end(), NodeTypePred< PortConnectionNode >());
	while (i != resolvedNodes.end())
	{
		const OutputPin* sourcePin = 0;
		std::vector< const InputPin* > destinationPins;

		for (RefArray< Edge >::iterator j = resolvedEdges.begin(); j != resolvedEdges.end(); )
		{
			if ((*j)->getSource()->getNode() == *i)
			{
				destinationPins.push_back((*j)->getDestination());
				j = resolvedEdges.erase(j);
			}
			else if ((*j)->getDestination()->getNode() == *i)
			{
				T_ASSERT (!sourcePin);
				sourcePin = (*j)->getSource();
				j = resolvedEdges.erase(j);
			}
			else
				++j;
		}

		if (sourcePin)
		{
			for (std::vector< const InputPin* >::const_iterator j = destinationPins.begin(); j != destinationPins.end(); ++j)
				resolvedEdges.push_back(new Edge(
					sourcePin,
					*j
				));
		}

		resolvedNodes.erase(i);
		i = std::find_if(resolvedNodes.begin(), resolvedNodes.end(), NodeTypePred< PortConnectionNode >());
	}

	// In case there still are unresolved edges to external nodes left then there is a mismatch
	// of external inputs and edges thus we need to fail.
	if (fullResolve)
	{
		std::map< const External*, std::set< std::wstring > > unresolvedExternalPins;
		for (RefArray< Edge >::const_iterator it = resolvedEdges.begin(); it != resolvedEdges.end(); ++it)
		{
			if (External* externalNode1 = dynamic_type_cast< External* >((*it)->getSource()->getNode()))
				unresolvedExternalPins[externalNode1].insert((*it)->getSource()->getName());
			else if (External* externalNode2 = dynamic_type_cast< External* >((*it)->getDestination()->getNode()))
				unresolvedExternalPins[externalNode2].insert((*it)->getDestination()->getName());
		}
		if (!unresolvedExternalPins.empty())
		{
			for (std::map< const External*, std::set< std::wstring > >::const_iterator it = unresolvedExternalPins.begin(); it != unresolvedExternalPins.end(); ++it)
			{
				if (optionalShaderGraphGuid)
					log::error << L"Fragment linkage of \"" << optionalShaderGraphGuid->format() << L"\" failed; unresolved edges, signature with fragment \"" << it->first->getFragmentGuid().format() << L"\" mismatch" << Endl;
				else
					log::error << L"Fragment linkage failed; unresolved edges, signature with fragment \"" << it->first->getFragmentGuid().format() << L"\" mismatch" << Endl;

				for (std::set< std::wstring >::const_iterator j = it->second.begin(); j != it->second.end(); ++j)
					log::error << L"\tPort \"" << *j << L"\" not defined" << Endl;
			}
			return 0;
		}
	}

	Ref< ShaderGraph > outputShaderGraph = new ShaderGraph(
		resolvedNodes,
		resolvedEdges
	);

#if defined(_DEBUG)
	validateShaderGraph(outputShaderGraph, fullResolve);
#endif

	return outputShaderGraph;
}

	}
}
