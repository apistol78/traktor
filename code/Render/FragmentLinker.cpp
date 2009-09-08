#include <map>
#include <algorithm>
#include "Render/FragmentLinker.h"
#include "Render/ShaderGraph.h"
#include "Render/Node.h"
#include "Render/Nodes.h"
#include "Render/External.h"
#include "Render/Edge.h"
#include "Core/Serialization/DeepClone.h"

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
	T_RTTI_CLASS(EstPort)

public:
	EstPort()
	:	ImmutableNode(c_EstPort_i, c_EstPort_o)
	{
	}
};

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.FragmentLinker.EstPort", EstPort, ImmutableNode)

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

ShaderGraph* FragmentLinker::resolve(const ShaderGraph* shaderGraph, bool fullResolve)
{
	Ref< ShaderGraph > resolvedShaderGraph = DeepClone(shaderGraph).create< ShaderGraph >();
	RefArray< Node > originalNodes = resolvedShaderGraph->getNodes();
	RefArray< Edge > resolvedEdges = resolvedShaderGraph->getEdges();
	RefArray< Node > resolvedNodes;

	for (RefArray< Node >::iterator i = originalNodes.begin(); i != originalNodes.end(); ++i)
	{
		External* externalNode = dynamic_type_cast< External* >(*i);
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

				std::map< std::wstring, EstPort* > inputEstPorts;
				std::map< std::wstring, EstPort* > outputEstPorts;

				for (int j = 0; j < externalNode->getInputPinCount(); ++j)
				{
					Ref< EstPort > inputEstPort = gc_new< EstPort >();
					inputEstPorts[externalNode->getInputPin(j)->getName()] = inputEstPort;
					resolvedNodes.push_back(inputEstPort);

					for (RefArray< Edge >::iterator k = resolvedEdges.begin(); k != resolvedEdges.end(); ++k)
					{
						if ((*k)->getDestination() == externalNode->getInputPin(j))
							(*k)->setDestination(inputEstPort->getInputPin(0));
					}
				}

				for (int j = 0; j < externalNode->getOutputPinCount(); ++j)
				{
					Ref< EstPort > outputEstPort = gc_new< EstPort >();
					outputEstPorts[externalNode->getOutputPin(j)->getName()] = outputEstPort;
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
						std::map< std::wstring, EstPort* >::iterator it = inputEstPorts.find(inputPort->getName());
						sourcePin = (it != inputEstPorts.end()) ? it->second->getOutputPin(0) : 0;
					}
					else
						sourcePin = (*j)->getSource();

					if (const OutputPort* outputPort = dynamic_type_cast< const OutputPort* >((*j)->getDestination()->getNode()))
					{
						std::map< std::wstring, EstPort* >::iterator it = outputEstPorts.find(outputPort->getName());
						destinationPin = (it != outputEstPorts.end()) ? it->second->getInputPin(0) : 0;
					}
					else
						destinationPin = (*j)->getDestination();

					if (sourcePin && destinationPin)
						resolvedEdges.push_back(gc_new< Edge >(sourcePin, destinationPin));
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
				resolvedEdges.push_back(gc_new< Edge >(sourcePin, *j));
		}

		resolvedNodes.erase(i);
	}

	return gc_new< ShaderGraph >(
		resolvedNodes,
		resolvedEdges
	);
}

	}
}
