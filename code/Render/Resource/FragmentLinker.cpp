#include <algorithm>
#include <map>
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

const ImmutableNode::InputPinDesc c_EstPort_i[] = { { L"Input", false }, 0 };
const ImmutableNode::OutputPinDesc c_EstPort_o[] = { L"Output", 0 };

class EstPort : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	EstPort()
	:	ImmutableNode(c_EstPort_i, c_EstPort_o)
	{
	}
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.FragmentLinker.EstPort", 0, EstPort, ImmutableNode)

template < typename NodeType >
struct NodeTypePred
{
	bool operator () (Node* node) const
	{
		return &type_of(node) == &type_of< NodeType >();
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

Ref< ShaderGraph > FragmentLinker::resolve(const ShaderGraph* shaderGraph, bool fullResolve)
{
	Ref< ShaderGraph > resolvedShaderGraph = DeepClone(shaderGraph).create< ShaderGraph >();
	RefArray< Node > originalNodes = resolvedShaderGraph->getNodes();
	RefArray< Edge > resolvedEdges = resolvedShaderGraph->getEdges();
	RefArray< Node > resolvedNodes;

	for (RefArray< Node >::iterator i = originalNodes.begin(); i != originalNodes.end(); ++i)
	{
		Ref< External > externalNode = dynamic_type_cast< External* >(*i);
		if (externalNode)
		{
			Ref< const ShaderGraph > fragmentShaderGraph = m_fragmentReader->read(externalNode->getFragmentGuid());
			if (fragmentShaderGraph)
			{
				if (fullResolve)
				{
					fragmentShaderGraph = resolve(fragmentShaderGraph, fullResolve);
					if (!fragmentShaderGraph)
						return 0;
				}

				std::map< std::wstring, Ref< Node > > inputEstablishedPorts;
				std::map< std::wstring, Ref< Node > > outputEstablishedPorts;

				RefArray< InputPort > fragmentInputPorts;
				fragmentShaderGraph->findNodesOf< InputPort >(fragmentInputPorts);

				for (RefArray< InputPort >::const_iterator j = fragmentInputPorts.begin(); j != fragmentInputPorts.end(); ++j)
				{
					std::wstring inputPortName = (*j)->getName();
					bool replaceWithValue = false;

					if ((*j)->isConnectable())
					{
						replaceWithValue = true;

						// Find input pin by input port.
						const InputPin* externalInputPin = externalNode->findInputPin(inputPortName);
						T_ASSERT (externalInputPin);

						// Create "established port" node and move all edges from input port to established port.
						Ref< EstPort > inputEstPort = new EstPort();
						for (RefArray< Edge >::iterator k = resolvedEdges.begin(); k != resolvedEdges.end(); ++k)
						{
							if ((*k)->getDestination() == externalInputPin)
							{
								(*k)->setDestination(inputEstPort->getInputPin(0));
								replaceWithValue = false;
							}
						}
						if (!replaceWithValue)
						{
							inputEstablishedPorts[inputPortName] = inputEstPort;
							resolvedNodes.push_back(inputEstPort);
						}
						else
							T_ASSERT_M (externalInputPin->isOptional(), L"Non-optional pin unconnected");
					}
					else
						replaceWithValue = true;

					if (replaceWithValue)
					{
						// Non-connected or value-only input port; replace with scalar value node.
						float value = externalNode->getValue(inputPortName, (*j)->getDefaultValue());
						Ref< Scalar > valueNode = new Scalar(value);

						inputEstablishedPorts[inputPortName] = valueNode;
						resolvedNodes.push_back(valueNode);
					}
				}

				for (int j = 0; j < externalNode->getOutputPinCount(); ++j)
				{
					Ref< EstPort > outputEstPort = new EstPort();
					outputEstablishedPorts[externalNode->getOutputPin(j)->getName()] = outputEstPort;
					resolvedNodes.push_back(outputEstPort);

					for (RefArray< Edge >::iterator k = resolvedEdges.begin(); k != resolvedEdges.end(); ++k)
					{
						if ((*k)->getSource() == externalNode->getOutputPin(j))
							(*k)->setSource(outputEstPort->getOutputPin(0));
					}
				}
				
				const RefArray< Edge >& fragmentEdges = fragmentShaderGraph->getEdges();
				for (RefArray< Edge >::const_iterator j = fragmentEdges.begin(); j != fragmentEdges.end(); ++j)
				{
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

				const RefArray< Node >& fragmentNodes = fragmentShaderGraph->getNodes();
				for (RefArray< Node >::const_iterator j = fragmentNodes.begin(); j != fragmentNodes.end(); ++j)
				{
					if (!is_a< InputPort >(*j) && !is_a< OutputPort >(*j))
						resolvedNodes.push_back(*j);
				}
			}
		}
		else
			resolvedNodes.push_back(*i);
	}

	for (;;)
	{
		RefArray< Node >::iterator i = std::find_if(resolvedNodes.begin(), resolvedNodes.end(), NodeTypePred< EstPort >());
		if (i == resolvedNodes.end())
			break;

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
	}

	return new ShaderGraph(
		resolvedNodes,
		resolvedEdges
	);
}

	}
}
