#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphValidator.h"
#include "Render/Editor/Shader/Node.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/Edge.h"
#include "Render/Editor/Shader/FragmentLinker.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_PortConnector_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_PortConnector_o[] = { { L"Output" }, { 0 } };

class PortConnector : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	PortConnector()
	:	ImmutableNode(c_PortConnector_i, c_PortConnector_o)
	{
	}
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.FragmentLinker.PortConnector", 0, PortConnector, ImmutableNode)

const InputPin* findExternalInputPin(const External* externalNode, const InputPort* fragmentInputPort)
{
	const int32_t inputPinCount = externalNode->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		const InputPin* inputPin = externalNode->getInputPin(i);
		if (
			inputPin->getId().isNotNull() &&
			inputPin->getId() == fragmentInputPort->getId()
		)
		{
			return inputPin;
		}
	}
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		const InputPin* inputPin = externalNode->getInputPin(i);
		if (
			inputPin->getId().isNull() &&
			inputPin->getName() == fragmentInputPort->getName()
		)
		{
			return inputPin;
		}
	}
	return nullptr;
}

const OutputPin* findExternalOutputPin(const External* externalNode, const OutputPort* fragmentOutputPort)
{
	const int32_t outputPinCount = externalNode->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		const OutputPin* outputPin = externalNode->getOutputPin(i);
		if (
			outputPin->getId().isNotNull() &&
			outputPin->getId() == fragmentOutputPort->getId()
		)
		{
			return outputPin;
		}
	}
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		const OutputPin* outputPin = externalNode->getOutputPin(i);
		if (
			outputPin->getId().isNull() &&
			outputPin->getName() == fragmentOutputPort->getName()
		)
		{
			return outputPin;
		}
	}
	return nullptr;
}

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
	std::wstring errorPrefix;
	if (optionalShaderGraphGuid)
		errorPrefix = L"Fragment linkage of \"" + optionalShaderGraphGuid->format() + L"\" failed; ";
	else
		errorPrefix = L"Fragment linkage failed; ";

	Ref< ShaderGraph > mutableShaderGraph = DeepClone(shaderGraph).create< ShaderGraph >();

	RefArray< External > externalNodes;
	mutableShaderGraph->findNodesOf< External >(externalNodes);
	for (auto externalNode : externalNodes)
	{
		Ref< const ShaderGraph > fragmentShaderGraph = m_fragmentReader->read(externalNode->getFragmentGuid());
		if (!fragmentShaderGraph)
		{
			log::error << errorPrefix << L"unable to read fragment \"" << externalNode->getFragmentGuid().format() << L"\"" << Endl;
			return nullptr;
		}
		if (fullResolve)
		{
			fragmentShaderGraph = resolve(fragmentShaderGraph, fullResolve, &externalNode->getFragmentGuid());
			if (!fragmentShaderGraph)
			{
				log::error << errorPrefix << L"unable to resolve fragment \"" << externalNode->getFragmentGuid().format() << L"\"" << Endl;
				return nullptr;
			}
		}

		// Move over fragment edges, transform input/output ports into temporary connectors, replace with values if necessary.
		for (auto edge : fragmentShaderGraph->getEdges())
		{
			if (!edge->getSource() || !edge->getDestination())
				continue;

			const OutputPin* sourcePin = nullptr;
			const InputPin* destinationPin = nullptr;

			if (const InputPort* inputPort = dynamic_type_cast< const InputPort* >(edge->getSource()->getNode()))
			{
				if (inputPort->isConnectable())
				{
					const InputPin* externalInputPin = findExternalInputPin(externalNode, inputPort);
					const OutputPin* externalSourcePin = mutableShaderGraph->findSourcePin(externalInputPin);
					if (externalInputPin && externalSourcePin)
					{
						Ref< PortConnector > connector = new PortConnector();
						mutableShaderGraph->addNode(connector);
						mutableShaderGraph->addEdge(new Edge(externalSourcePin, connector->getInputPin(0)));
						sourcePin = connector->getOutputPin(0);
					}
					else if (inputPort->isOptional())
					{
						Ref< Scalar > scalarNode;

						const auto& values = externalNode->getValues();
						auto it = values.find(inputPort->getId().format());
						if (it == values.end())
							it = values.find(inputPort->getName());
						if (it != values.end())
							scalarNode = new Scalar(it->second);
						else if (inputPort->haveDefaultValue())
							scalarNode = new Scalar(inputPort->getDefaultValue());

						if (scalarNode)
						{
							mutableShaderGraph->addNode(scalarNode);
							sourcePin = scalarNode->getOutputPin(0);
						}
					}
					else
					{
						log::error << errorPrefix << L"mandatory input \"" << inputPort->getName() << L"\" of fragment \"" << externalNode->getFragmentGuid().format() << L"\" not connected." << Endl;
						return nullptr;
					}
				}
				else
				{
					Ref< Scalar > scalarNode;

					const auto& values = externalNode->getValues();
					auto it = values.find(inputPort->getId().format());
					if (it == values.end())
						it = values.find(inputPort->getName());
					if (it != values.end())
						scalarNode = new Scalar(it->second);
					else if (inputPort->haveDefaultValue())
						scalarNode = new Scalar(inputPort->getDefaultValue());

					if (scalarNode)
					{
						mutableShaderGraph->addNode(scalarNode);
						sourcePin = scalarNode->getOutputPin(0);
					}
				}
			}
			else
				sourcePin = edge->getSource();

			if (const OutputPort* outputPort = dynamic_type_cast< const OutputPort* >(edge->getDestination()->getNode()))
			{
				const OutputPin* externalOutputPin = findExternalOutputPin(externalNode, outputPort);
				
				std::vector< const InputPin* > externalDestinationPins;
				mutableShaderGraph->findDestinationPins(externalOutputPin, externalDestinationPins);

				if (externalOutputPin && !externalDestinationPins.empty())
				{
					Ref< PortConnector > connector = new PortConnector();
					mutableShaderGraph->addNode(connector);
					for (auto externalDestinationPin : externalDestinationPins)
						mutableShaderGraph->addEdge(new Edge(connector->getOutputPin(0), externalDestinationPin));
					destinationPin = connector->getInputPin(0);
				}
			}
			else
				destinationPin = edge->getDestination();

			if (sourcePin && destinationPin)
				mutableShaderGraph->addEdge(new Edge(
					sourcePin,
					destinationPin
				));
		}

		// Move over all non-port nodes from fragment as-is.
		for (auto node : fragmentShaderGraph->getNodes())
		{
			if (!is_a< InputPort >(node) && !is_a< OutputPort >(node))
				mutableShaderGraph->addNode(node);
		}

		// Remove external node.
		mutableShaderGraph->detach(externalNode);
		mutableShaderGraph->removeNode(externalNode);
	}

	// Re-wire edges which has been temporarily connected to a "port connection" node.
	RefArray< PortConnector > connectors;
	mutableShaderGraph->findNodesOf< PortConnector >(connectors);
	for (auto connector : connectors)
	{
		const OutputPin* sourcePin = mutableShaderGraph->findSourcePin(connector->getInputPin(0));

		std::vector< const InputPin* > destinationPins;		
		mutableShaderGraph->findDestinationPins(connector->getOutputPin(0), destinationPins);

		mutableShaderGraph->detach(connector);
		mutableShaderGraph->removeNode(connector);

		for (auto destinationPin : destinationPins)
		{
			mutableShaderGraph->addEdge(new Edge(
				sourcePin,
				destinationPin
			));
		}
	}

	return mutableShaderGraph;
}

	}
}
