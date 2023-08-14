/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphStatic.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphValidator.h"

namespace traktor::render
{
	namespace
	{

#define T_VALIDATE_SHADERGRAPH(sg) // T_FATAL_ASSERT(ShaderGraphValidator(sg).validateIntegrity())

const ImmutableNode::InputPinDesc c_PortConnector_i[] = { { L"Input", L"{581D486E-5A7F-46CE-BB38-251008DCF746}", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_PortConnector_o[] = { { L"Output", L"{255B6D44-455E-46D2-9865-E1764FBF20BF}" }, { 0 } };

class PortConnector : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	explicit PortConnector(const Guid& fromFragmentId)
	:	ImmutableNode(c_PortConnector_i, c_PortConnector_o)
	,	m_fromFragmentId(fromFragmentId)
	{
	}

private:
	Guid m_fromFragmentId;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.FragmentLinker.PortConnector", PortConnector, ImmutableNode)

const InputPin* findExternalInputPin(const External* externalNode, const InputPort* fragmentInputPort)
{
	const int32_t inputPinCount = externalNode->getInputPinCount();
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		const InputPin* inputPin = externalNode->getInputPin(i);
		if (inputPin->getName() == fragmentInputPort->getName())
			return inputPin;
	}
	return nullptr;
}

const OutputPin* findExternalOutputPin(const External* externalNode, const OutputPort* fragmentOutputPort)
{
	const int32_t outputPinCount = externalNode->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		const OutputPin* outputPin = externalNode->getOutputPin(i);
		if (outputPin->getName() == fragmentOutputPort->getName())
			return outputPin;
	}
	return nullptr;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.FragmentLinker", FragmentLinker, Object)

FragmentLinker::FragmentLinker(const IFragmentReader& fragmentReader)
:	m_fragmentReader(&fragmentReader)
,	m_own(false)
{
}

FragmentLinker::FragmentLinker(const std::function< Ref< const ShaderGraph >(const Guid&) >& fragmentReader)
:	m_fragmentReader(new LambdaFragmentReader(fragmentReader))
,	m_own(true)
{
}

FragmentLinker::~FragmentLinker()
{
	if (m_own)
		delete m_fragmentReader;
}

Ref< ShaderGraph > FragmentLinker::resolve(const ShaderGraph* shaderGraph, bool fullResolve, const Guid* optionalShaderGraphGuid) const
{
	const RefArray< External > externalNodes = shaderGraph->findNodesOf< External >();
	return resolve(shaderGraph, externalNodes, fullResolve, optionalShaderGraphGuid);
}

Ref< ShaderGraph > FragmentLinker::resolve(const ShaderGraph* shaderGraph, const RefArray< External >& externalNodes, bool fullResolve, const Guid* optionalShaderGraphGuid) const
{
	std::wstring errorPrefix;
	if (optionalShaderGraphGuid)
		errorPrefix = L"Fragment linkage of \"" + optionalShaderGraphGuid->format() + L"\" failed; ";
	else
		errorPrefix = L"Fragment linkage failed; ";

	Ref< ShaderGraph > mutableShaderGraph = new ShaderGraph(
		shaderGraph->getNodes(),
		shaderGraph->getEdges()
	);
	T_VALIDATE_SHADERGRAPH(mutableShaderGraph);

	for (auto externalNode : externalNodes)
	{
		const Guid& fragmentId = externalNode->getFragmentGuid();

		// Read fragment shader.
		Ref< const ShaderGraph > fragmentShaderGraph = m_fragmentReader->read(fragmentId);
		if (!fragmentShaderGraph)
		{
			log::error << errorPrefix << L"unable to read fragment \"" << fragmentId.format() << L"\"." << Endl;
			return nullptr;
		}
		T_VALIDATE_SHADERGRAPH(fragmentShaderGraph);

		// Resolve variables of each read fragment.
		fragmentShaderGraph = ShaderGraphStatic(fragmentShaderGraph, fragmentId).getVariableResolved();
		if (!fragmentShaderGraph)
		{
			log::error << errorPrefix << L"unable to resolve variables in fragment \"" << fragmentId.format() << L"\"" << Endl;
			return nullptr;
		}

		// Recursive resolve fragments if we need a full resolve.
		if (fullResolve)
		{
			fragmentShaderGraph = resolve(fragmentShaderGraph, true, &fragmentId);
			if (!fragmentShaderGraph)
			{
				log::error << errorPrefix << L"unable to resolve fragment \"" << fragmentId.format() << L"\"" << Endl;
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
						Ref< PortConnector > connector = new PortConnector(fragmentId);
						mutableShaderGraph->addNode(connector);
						mutableShaderGraph->addEdge(new Edge(externalSourcePin, connector->getInputPin(0)));
						sourcePin = connector->getOutputPin(0);
					}
					else if (inputPort->isOptional())
					{
						Ref< Scalar > scalarNode;

						const auto& values = externalNode->getValues();
						auto it = values.find(inputPort->getName());
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
						log::error << errorPrefix << L"mandatory input port \"" << inputPort->getName() << L"\" of fragment \"" << fragmentId.format() << L"\" not connected." << Endl;
						return nullptr;
					}
				}
				else
				{
					Ref< Scalar > scalarNode;

					const auto& values = externalNode->getValues();
					auto it = values.find(inputPort->getName());
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
				RefArray< Edge > externalDestinationEdges = mutableShaderGraph->findEdges(externalOutputPin);

				if (externalOutputPin && !externalDestinationEdges.empty())
				{
					Ref< PortConnector > connector = new PortConnector(fragmentId);
					mutableShaderGraph->addNode(connector);
					for (auto externalDestinationEdge : externalDestinationEdges)
					{
						mutableShaderGraph->removeEdge(externalDestinationEdge);
						mutableShaderGraph->addEdge(new Edge(
							connector->getOutputPin(0),
							externalDestinationEdge->getDestination()
						));
					}
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
		mutableShaderGraph->removeNode(externalNode);
		T_VALIDATE_SHADERGRAPH(mutableShaderGraph);
	}

	// Re-wire edges which has been temporarily connected to a "port connection" node.
	for (auto connector : mutableShaderGraph->findNodesOf< PortConnector >())
	{
		const OutputPin* sourcePin = mutableShaderGraph->findSourcePin(connector->getInputPin(0));
		if (!sourcePin)
		{
			mutableShaderGraph->removeNode(connector);
			continue;
		}
		
		AlignedVector< const InputPin* > destinationPins = mutableShaderGraph->findDestinationPins(connector->getOutputPin(0));
		for (auto destinationPin : destinationPins)
		{
			T_FATAL_ASSERT(destinationPin->getNode() != sourcePin->getNode());
			mutableShaderGraph->addEdge(new Edge(
				sourcePin,
				destinationPin
			));
		}

		mutableShaderGraph->removeNode(connector);
	}

	// Create a unique clone of the resolved shader before returning.
	mutableShaderGraph = DeepClone(mutableShaderGraph).create< ShaderGraph >();
	T_VALIDATE_SHADERGRAPH(mutableShaderGraph);

	return mutableShaderGraph;
}

}
