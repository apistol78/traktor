/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma optimize( "", off )

#include <cctype>
#include "Core/Containers/AlignedVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/ImmutableCheck.h"
#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/GraphTraverse.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"
#include "Render/Editor/Shader/ShaderGraphTypeEvaluator.h"
#include "Render/Editor/Shader/ShaderGraphTypePropagation.h"
#include "Render/Editor/Shader/ShaderGraphValidator.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct ConstantFoldingVisitor
{
	Ref< ShaderGraph > m_shaderGraph;
	const SmallMap< const OutputPin*, Constant >& m_outputConstants;

	ConstantFoldingVisitor(ShaderGraph* shaderGraph, const SmallMap< const OutputPin*, Constant >& outputConstants)
	:	m_shaderGraph(shaderGraph)
	,	m_outputConstants(outputConstants)
	{
	}

	bool operator () (Node* node)
	{
		m_shaderGraph->addNode(node);
		return true;
	}

	bool operator () (Edge* edge)
	{
		const OutputPin* source = edge->getSource();

		SmallMap< const OutputPin*, Constant >::const_iterator it = m_outputConstants.find(source);
		if (it != m_outputConstants.end() && it->second.isAllConst())
		{
			std::pair< int, int > position = source->getNode()->getPosition();
			Ref< Node > node;

			switch (it->second.getType())
			{
			case PinType::Scalar1:
				node = new Scalar(it->second.x());
				break;

			case PinType::Scalar2:
				{
					Ref< Vector > value = new Vector(Vector4(it->second.x(), it->second.y(), 0.0f, 0.0f));
					Ref< Swizzle > swizzle = new Swizzle(L"xy");

					m_shaderGraph->addNode(value);
					m_shaderGraph->addEdge(new Edge(value->getOutputPin(0), swizzle->getInputPin(0)));

					value->setPosition(position);
					position.first += 32;

					node = swizzle;
				}
				break;

			case PinType::Scalar3:
				{
					Ref< Vector > value = new Vector(Vector4(it->second.x(), it->second.y(), it->second.z(), 0.0f));
					Ref< Swizzle > swizzle = new Swizzle(L"xyz");

					m_shaderGraph->addNode(value);
					m_shaderGraph->addEdge(new Edge(value->getOutputPin(0), swizzle->getInputPin(0)));

					value->setPosition(position);
					position.first += 32;

					node = swizzle;
				}
				break;

			case PinType::Scalar4:
				node = new Vector(Vector4(it->second.x(), it->second.y(), it->second.z(), it->second.w()));
				break;

			default:
				break;
			}

			if (node)
			{
				node->setPosition(position);
				m_shaderGraph->addNode(node);
				m_shaderGraph->addEdge(new Edge(node->getOutputPin(0), edge->getDestination()));
				return false;
			}
		}

		m_shaderGraph->addEdge(edge);
		return true;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphStatic", ShaderGraphStatic, Object)

ShaderGraphStatic::ShaderGraphStatic(const ShaderGraph* shaderGraph, const Guid& shaderGraphId)
{
	T_ASSERT(ShaderGraphValidator(shaderGraph, shaderGraphId).validateIntegrity());
	m_shaderGraph = shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getPlatformPermutation(const std::wstring& platform) const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	RefArray< Platform > nodes;
	shaderGraph->findNodesOf< Platform >(nodes);

	for (const auto node : nodes)
	{
		const InputPin* inputPin = node->findInputPin(platform);
		T_ASSERT(inputPin);

		Ref< Edge > sourceEdge = shaderGraph->findEdge(inputPin);
		if (!sourceEdge)
		{
			inputPin = node->findInputPin(L"Other");
			T_ASSERT(inputPin);

			sourceEdge = shaderGraph->findEdge(inputPin);
			if (!sourceEdge)
				return nullptr;
		}

		const OutputPin* outputPin = node->findOutputPin(L"Output");
		T_ASSERT(outputPin);

		RefSet< Edge > destinationEdges;
		shaderGraph->findEdges(outputPin, destinationEdges);

		shaderGraph->removeEdge(sourceEdge);
		for (const auto destinationEdge : destinationEdges)
		{
			shaderGraph->removeEdge(destinationEdge);
			shaderGraph->addEdge(new Edge(
				sourceEdge->getSource(),
				destinationEdge->getDestination()
			));
		}

		shaderGraph->detach(node);
		shaderGraph->removeNode(node);
	}

	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getRendererPermutation(const std::wstring& renderer) const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	RefArray< Renderer > nodes;
	shaderGraph->findNodesOf< Renderer >(nodes);

	for (const auto node : nodes)
	{
		const InputPin* inputPin = node->findInputPin(renderer);
		T_ASSERT(inputPin);

		Ref< Edge > sourceEdge = shaderGraph->findEdge(inputPin);
		if (!sourceEdge)
		{
			inputPin = node->findInputPin(L"Other");
			T_ASSERT(inputPin);

			sourceEdge = shaderGraph->findEdge(inputPin);
			if (!sourceEdge)
				return nullptr;
		}

		const OutputPin* outputPin = node->findOutputPin(L"Output");
		T_ASSERT(outputPin);

		RefSet< Edge > destinationEdges;
		shaderGraph->findEdges(outputPin, destinationEdges);

		shaderGraph->removeEdge(sourceEdge);
		for (const auto destinationEdge : destinationEdges)
		{
			shaderGraph->removeEdge(destinationEdge);
			shaderGraph->addEdge(new Edge(
				sourceEdge->getSource(),
				destinationEdge->getDestination()
			));
		}

		shaderGraph->detach(node);
		shaderGraph->removeNode(node);
	}

	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getConnectedPermutation() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	RefArray< Connected > nodes;
	if (shaderGraph->findNodesOf< Connected >(nodes) <= 0)
		return shaderGraph;

	for (const auto node : nodes)
	{
		const InputPin* inputPin = node->findInputPin(L"Input");
		T_ASSERT(inputPin);

		bool inputConnected = (bool)(shaderGraph->findEdge(inputPin) != nullptr);

		inputPin = node->findInputPin(inputConnected ? L"True" : L"False");
		T_ASSERT(inputPin);

		const OutputPin* sourceOutputPin = nullptr;

		Ref< Edge > sourceEdge = shaderGraph->findEdge(inputPin);
		if (sourceEdge)
		{
			sourceOutputPin = sourceEdge->getSource();
			shaderGraph->removeEdge(sourceEdge);
		}
		else
		{
			Ref< Scalar > scalarNode = new Scalar(inputConnected ? 1.0f : 0.0f);
			shaderGraph->addNode(scalarNode);
			sourceOutputPin = scalarNode->getOutputPin(0);
		}

		const OutputPin* outputPin = node->findOutputPin(L"Output");
		T_ASSERT(outputPin);

		RefSet< Edge > destinationEdges;
		shaderGraph->findEdges(outputPin, destinationEdges);

		for (const auto destinationEdge : destinationEdges)
		{
			shaderGraph->removeEdge(destinationEdge);
			shaderGraph->addEdge(new Edge(
				sourceOutputPin,
				destinationEdge->getDestination()
			));
		}

		shaderGraph->detach(node);
		shaderGraph->removeNode(node);
	}

	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getTypePermutation() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	RefArray< Type > nodes;
	if (shaderGraph->findNodesOf< Type >(nodes) <= 0)
		return shaderGraph;

	ShaderGraphTypeEvaluator evaluator(shaderGraph);
	for (const auto node : nodes)
	{
		const OutputPin* outputPin = node->findOutputPin(L"Output");
		T_ASSERT(outputPin);

		PinType inputType = evaluator.evaluate(node, L"Type");

		const InputPin* inputPin = nullptr;

		if (isPinTypeScalar(inputType))
		{
			if (getPinTypeWidth(inputType) <= 1)
				inputPin = node->findInputPin(L"Scalar");
			else
				inputPin = node->findInputPin(L"Vector");
		}
		else if (inputType == PinType::Matrix)
		{
			inputPin = node->findInputPin(L"Matrix");
		}
		else if (isPinTypeTexture(inputType))
		{
			inputPin = node->findInputPin(L"Texture");
		}
		else if (isPinTypeState(inputType))
		{
			inputPin = node->findInputPin(L"State");
		}
		else if (inputType == PinType::Void)
		{
			inputPin = node->findInputPin(L"Default");
		}

		if (!inputPin)
			return nullptr;

		Ref< Edge > sourceEdge = shaderGraph->findEdge(inputPin);
		if (!sourceEdge)
		{
			inputPin = node->findInputPin(L"Default");
			T_ASSERT(inputPin);

			sourceEdge = shaderGraph->findEdge(inputPin);
			if (!sourceEdge)
				return nullptr;
		}

		RefSet< Edge > destinationEdges;
		shaderGraph->findEdges(outputPin, destinationEdges);

		shaderGraph->removeEdge(sourceEdge);
		for (const auto destinationEdge : destinationEdges)
		{
			shaderGraph->removeEdge(destinationEdge);
			shaderGraph->addEdge(new Edge(
				sourceEdge->getSource(),
				destinationEdge->getDestination()
			));
		}
	}

	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getSwizzledPermutation() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	ShaderGraphTypePropagation typePropagation(shaderGraph);

	// Each output are now assigned a required output width.
	// Replace all edges with a "o--[swizzle]--o" pattern.
	const wchar_t* c_swizzles[] = { L"x", L"xy", L"xyz", L"xyzw" };

	RefArray< Edge > edges = shaderGraph->getEdges();
	for (auto edge : edges)
	{
		const OutputPin* sourcePin = edge->getSource();
		const InputPin* destinationPin = edge->getDestination();

		PinType sourceType = typePropagation.evaluate(sourcePin);
		PinType destinationType = typePropagation.evaluate(destinationPin);

		if (sourceType <= destinationType || !isPinTypeScalar(sourceType) || !isPinTypeScalar(destinationType))
			continue;

		// \fixme Bug in shader graph; must remove edge prior to adding new edges.
		shaderGraph->removeEdge(edge);

		const auto p1 = sourcePin->getNode()->getPosition();
		const auto p2 = destinationPin->getNode()->getPosition();

		std::pair< int, int > p(
			(p1.first + p2.first) / 2,
			(p1.second + p2.second) / 2
		);

		Ref< Swizzle > swizzleNode = new Swizzle(c_swizzles[getPinTypeWidth(destinationType) - 1]);
		swizzleNode->setPosition(p);

		Ref< Edge > edgeIn = new Edge(sourcePin, swizzleNode->getInputPin(0));
		Ref< Edge > edgeOut = new Edge(swizzleNode->getOutputPin(0), destinationPin);

		shaderGraph->addNode(swizzleNode);
		shaderGraph->addEdge(edgeIn);
		shaderGraph->addEdge(edgeOut);
	}

	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getConstantFolded() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	SmallMap< const OutputPin*, Constant > outputConstants;
	AlignedVector< Constant > inputConstants;
	AlignedVector< const OutputPin* > inputOutputPins;
	RefSet< Edge > edges;

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	ShaderGraphTypePropagation typePropagation(shaderGraph);
	const RefArray< Node >& nodes = shaderGraph->getNodes();

	// Setup map of all output pin "constants"; each such constant has proper width and
	// are initially set to be completely variant.
	for (const auto node : nodes)
	{
		const INodeTraits* nodeTraits = INodeTraits::find(node);
		T_FATAL_ASSERT (nodeTraits);

		int32_t outputPinCount = node->getOutputPinCount();
		for (int32_t i = 0; i < outputPinCount; ++i)
		{
			const OutputPin* outputPin = node->getOutputPin(i);
			T_ASSERT(outputPin);

			PinType outputPinType = typePropagation.evaluate(outputPin);
			outputConstants[outputPin] = Constant(outputPinType);
		}
	}

	// Keep on iterating until no constants has been propagated.
	uint32_t constantPropagationCount;
	do
	{
restart_iteration:

		constantPropagationCount = 0;

		for (const auto node : nodes)
		{
			const INodeTraits* nodeTraits = INodeTraits::find(node);
			T_ASSERT(nodeTraits);

			int32_t inputPinCount = node->getInputPinCount();

			// Get set of input constants and source pins.
			inputConstants.resize(inputPinCount);
			inputOutputPins.resize(inputPinCount);
			for (int32_t j = 0; j < inputPinCount; ++j)
			{
				const InputPin* inputPin = node->getInputPin(j);
				T_ASSERT(inputPin);

				inputConstants[j] = Constant();
				inputOutputPins[j] = nullptr;

				const OutputPin* outputPin = shaderGraph->findSourcePin(inputPin);
				if (outputPin)
				{
					inputOutputPins[j] = outputPin;

					PinType inputPinType = typePropagation.evaluate(inputPin);
					if (isPinTypeScalar(inputPinType))
					{
						SmallMap< const OutputPin*, Constant >::const_iterator it = outputConstants.find(outputPin);
						T_FATAL_ASSERT (it != outputConstants.end());

						inputConstants[j] = it->second.cast(inputPinType);
					}
				}
			}

			// Evaluate result of all output pins.
			int32_t outputPinCount = node->getOutputPinCount();
			for (int32_t j = 0; j < outputPinCount; ++j)
			{
				const OutputPin* outputPin = node->getOutputPin(j);
				T_ASSERT(outputPin);

				// Don't evaluate output if it's not connected to anything.
				if (shaderGraph->getDestinationCount(outputPin) <= 0)
					continue;

				PinType outputPinType = typePropagation.evaluate(outputPin);

				// First attempt to evaluate re-wiring to circumvent this node entirely.
				if (inputPinCount > 0)
				{
					const OutputPin* foldOutputPin = nullptr;
					if (nodeTraits->evaluatePartial(
						shaderGraph,
						node,
						outputPin,
						&inputOutputPins[0],
						&inputConstants[0],
						foldOutputPin
					))
					{
						Ref< Swizzle > swizzleNode;

						PinType outputPinType = typePropagation.evaluate(outputPin);
						PinType foldOutputPinType = typePropagation.evaluate(foldOutputPin);

						if (isPinTypeScalar(outputPinType) && isPinTypeScalar(foldOutputPinType) && outputPinType != foldOutputPinType)
						{
							// It is possible "folded output pin"'s type is wider than "output pin" type due to
							// hard constraints on type propagation (ex. VertexInput cannot change type).

							if (foldOutputPinType == PinType::Scalar1)
							{
								const wchar_t* c_scalarSwizzles[] = { L"", L"xx", L"xxx", L"xxxx" };
								swizzleNode = new Swizzle(c_scalarSwizzles[getPinTypeWidth(outputPinType) - 1]);
							}
							else if (foldOutputPinType == PinType::Scalar2)
							{
								const wchar_t* c_tupleSwizzles[] = { L"x", L"", L"xy0", L"xy00" };
								swizzleNode = new Swizzle(c_tupleSwizzles[getPinTypeWidth(outputPinType) - 1]);
							}
							else if (foldOutputPinType == PinType::Scalar3)
							{
								const wchar_t* c_tripleSwizzles[] = { L"x", L"xy", L"", L"xyz0" };
								swizzleNode = new Swizzle(c_tripleSwizzles[getPinTypeWidth(outputPinType) - 1]);
							}
							else if (foldOutputPinType == PinType::Scalar4)
							{
								const wchar_t* c_tripleSwizzles[] = { L"x", L"xy", L"xyz", L"" };
								swizzleNode = new Swizzle(c_tripleSwizzles[getPinTypeWidth(outputPinType) - 1]);
							}

							if (swizzleNode)
							{
								Ref< Edge > edgeIn = new Edge(foldOutputPin, swizzleNode->getInputPin(0));

								shaderGraph->addEdge(edgeIn);
								shaderGraph->addNode(swizzleNode);

								foldOutputPin = swizzleNode->getOutputPin(0);

								// Update types in type evaluator.
								typePropagation.set(swizzleNode->getInputPin(0), outputPinType);
								typePropagation.set(swizzleNode->getOutputPin(0), outputPinType);

								// Add output pin of new swizzle node to known set.
								outputConstants[foldOutputPin] = outputConstants[outputPin];
							}
						}

						edges.reset();
						shaderGraph->findEdges(outputPin, edges);
						for (auto edge : edges)
						{
							shaderGraph->removeEdge(edge);
							shaderGraph->addEdge(new Edge(foldOutputPin, edge->getDestination()));
						}

						// We need to restart iteration if nodes has been added to iteration set.
						if (swizzleNode)
							goto restart_iteration;
						else
							constantPropagationCount++;
					}
				}

				// Then attempt to evaluate as much output as possible.
				Constant outputConstant(outputPinType);
				if (nodeTraits->evaluatePartial(
					shaderGraph,
					node,
					outputPin,
					inputPinCount > 0 ? &inputConstants[0] : nullptr,
					outputConstant
				))
				{
					// If the output of the pin has changed then a propagation must have taken place.
					if (outputConstants[outputPin] != outputConstant)
					{
						constantPropagationCount++;
						outputConstants[outputPin] = outputConstant;
					}
				}
			}
		}
	}
	while (constantPropagationCount > 0);

	// Collect root nodes.
	RefArray< Node > roots;
	for (const auto node : nodes)
	{
		const INodeTraits* traits = INodeTraits::find(node);
		T_FATAL_ASSERT (traits);

		if (traits->isRoot(shaderGraph, node))
			roots.push_back(node);
	}

	// Traverse copy shader graph; replace inputs if constant.
	ConstantFoldingVisitor visitor(new ShaderGraph(), outputConstants);
	GraphTraverse(shaderGraph, roots).preorder(visitor);

	T_ASSERT(ShaderGraphValidator(visitor.m_shaderGraph).validateIntegrity());
	return visitor.m_shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::cleanupRedundantSwizzles() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();

	RefArray< Swizzle > swizzleNodes;
	shaderGraph->findNodesOf< Swizzle >(swizzleNodes);
	for (auto i = swizzleNodes.begin(); i != swizzleNodes.end(); )
	{
		Swizzle* swizzleRightNode = *i;
		T_ASSERT(swizzleRightNode);

		const InputPin* swizzleInput = swizzleRightNode->getInputPin(0);
		T_ASSERT(swizzleInput);

		const OutputPin* swizzleOutput = swizzleRightNode->getOutputPin(0);
		T_ASSERT(swizzleOutput);

		Edge* sourceEdge = shaderGraph->findEdge(swizzleInput);
		if (!sourceEdge || !sourceEdge->getSource())
		{
			++i;
			continue;
		}

		// Get left swizzle; cast to null if input ain't a swizzle node.
		Swizzle* swizzleLeftNode = dynamic_type_cast< Swizzle* >(sourceEdge->getSource()->getNode());
		if (!swizzleLeftNode)
		{
			++i;
			continue;
		}

		// Merge swizzle patterns.
		std::wstring swizzleRight = swizzleRightNode->get();
		std::wstring swizzleLeft = swizzleLeftNode->get();

		int32_t swizzleRightWidth = swizzleRight.length();
		int32_t swizzleLeftWidth = swizzleLeft.length();

		std::wstring swizzle = swizzleRight;

		for (int32_t j = 0; j < swizzleRightWidth; ++j)
		{
			wchar_t sr = std::tolower(swizzleRight[j]);
			size_t sri = std::wstring(L"xyzw").find(sr);
			if (sri != std::wstring::npos)
			{
				wchar_t s = sri < swizzleLeftWidth ? std::tolower(swizzleLeft[sri]) : L'0';
				swizzle[j] = s;
			}
		}

		swizzleRightNode->set(swizzle);

		// Replace input edge with edge from left's source.
		shaderGraph->removeEdge(sourceEdge);

		const OutputPin* sourcePin = shaderGraph->findSourcePin(swizzleLeftNode->getInputPin(0));
		if (sourcePin != nullptr)
		{
			shaderGraph->addEdge(new Edge(
				sourcePin,
				swizzleRightNode->getInputPin(0)
			));
		}

		T_ASSERT(ShaderGraphValidator(shaderGraph).validateIntegrity());

		// Restart iteration as it's possible we have rewired to another swizzler.
		i = swizzleNodes.begin();
	}

	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getStateResolved() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	// Pixel output state.
	RefArray< PixelOutput > pixelOutputNodes;
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputNodes);
	for (const auto pixelOutputNode : pixelOutputNodes)
	{
		Ref< Edge > edge = shaderGraph->findEdge(pixelOutputNode->findInputPin(L"State"));
		if (!edge)
			continue;

		Ref< PixelState > state = dynamic_type_cast< PixelState* >(edge->getSource()->getNode());
		if (!state)
			continue;

		Ref< PixelOutput > resolvedPixelOutput = new PixelOutput();
		resolvedPixelOutput->setTechnique(pixelOutputNode->getTechnique());
		resolvedPixelOutput->setPriority(state->getPriority());
		resolvedPixelOutput->setRenderState(state->getRenderState());
		resolvedPixelOutput->setRegisterCount(pixelOutputNode->getRegisterCount());
		resolvedPixelOutput->setPrecisionHint(state->getPrecisionHint());

		shaderGraph->removeEdge(edge);
		shaderGraph->removeNode(state);
		shaderGraph->replace(pixelOutputNode, resolvedPixelOutput);

		T_ASSERT(ShaderGraphValidator(shaderGraph).validateIntegrity());
	}

	// Sampler state.
	RefArray< Sampler > samplerNodes;
	shaderGraph->findNodesOf< Sampler >(samplerNodes);
	for (const auto samplerNode : samplerNodes)
	{
		Ref< Edge > edge = shaderGraph->findEdge(samplerNode->findInputPin(L"State"));
		if (!edge)
			continue;

		Ref< TextureState > state = dynamic_type_cast< TextureState* >(edge->getSource()->getNode());
		if (!state)
			continue;

		Ref< Sampler > resolvedSampler = new Sampler();
		resolvedSampler->setSamplerState(state->getSamplerState());

		shaderGraph->removeEdge(edge);
		shaderGraph->removeNode(state);
		shaderGraph->replace(samplerNode, resolvedSampler);

		T_ASSERT(ShaderGraphValidator(shaderGraph).validateIntegrity());
	}

	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getVariableResolved(VariableResolveType resolve) const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();

	// Get all variable nodes from shader graph.
	RefArray< Variable > variableNodes;
	shaderGraph->findNodesOf< Variable >(variableNodes);

	// Ignore variables of other scopes.
	for (;;)
	{
		auto it = std::find_if(variableNodes.begin(), variableNodes.end(), [&](const Variable* variableNode) {
			return (resolve == VrtLocal && variableNode->isGlobal()) || (resolve == VrtGlobal && !variableNode->isGlobal());
		});
		if (it != variableNodes.end())
			variableNodes.erase(it);
		else
			break;
	}

	// Join variable references.
	for (const auto variableNode : variableNodes)
	{
		const OutputPin* variableOutput = variableNode->getOutputPin(0);
		T_ASSERT(variableOutput);

		if (shaderGraph->getDestinationCount(variableOutput) > 0)
		{
			auto it = std::find_if(variableNodes.begin(), variableNodes.end(), [&](const Variable* node) {
				if (node->getName() != variableNode->getName())
					return false;
				// Matching names, is this variable written to?
				return shaderGraph->findSourcePin(node->getInputPin(0)) != nullptr;
			});
			if (it != variableNodes.end())
			{
				const InputPin* variableInput = (*it)->getInputPin(0);
				T_ASSERT(variableInput);

				const OutputPin* sourcePin = shaderGraph->findSourcePin(variableInput);
				T_ASSERT(sourcePin);

				shaderGraph->rewire(variableOutput, sourcePin);
			}
			else
			{
				// Variable is undefined, disconnect output.
				shaderGraph->rewire(variableOutput, nullptr);
			}
		}
	}

	// Remove all variables.
	for (const auto variableNode : variableNodes)
	{
		shaderGraph->detach(variableNode);
		shaderGraph->removeNode(variableNode);
	};

	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::removeDisabledOutputs() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	RefArray< PixelOutput > pixelOutputNodes;
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputNodes);

	for (const auto pixelOutputNode : pixelOutputNodes)
	{
		const OutputPin* enableSource = shaderGraph->findSourcePin(pixelOutputNode->getInputPin(0));
		if (!enableSource)
		{
			// Keep outputs with unconnected "Enable" pin, enabled by default.
			continue;
		}

		const Scalar* scalar = dynamic_type_cast< const Scalar* >(enableSource->getNode());
		if (scalar)
		{
			if (scalar->get() < FUZZY_EPSILON)
			{
				// Zero as input to "Enable" pin, remove output.
				shaderGraph->detach(pixelOutputNode);
				shaderGraph->removeNode(pixelOutputNode);
			}
		}
		else if (!is_a< InputPort >(enableSource->getNode()))
			log::warning << L"Unsupported node type of input; Only Scalar nodes can be connected to \"Enable\" of \"traktor.render.PixelOutput\". " << type_name(enableSource->getNode()) << L" not supported." << Endl;
	}

	RefArray< ComputeOutput > computeOutputNodes;
	shaderGraph->findNodesOf< ComputeOutput >(computeOutputNodes);

	for (const auto computeOutputNode : computeOutputNodes)
	{
		const OutputPin* enableSource = shaderGraph->findSourcePin(computeOutputNode->getInputPin(0));
		if (!enableSource)
		{
			// Keep outputs with unconnected "Enable" pin, enabled by default.
			continue;
		}

		const Scalar* scalar = dynamic_type_cast< const Scalar* >(enableSource->getNode());
		if (scalar)
		{
			if (scalar->get() < FUZZY_EPSILON)
			{
				// Zero as input to "Enable" pin, remove output.
				shaderGraph->detach(computeOutputNode);
				shaderGraph->removeNode(computeOutputNode);
			}
		}
		else if (!is_a< InputPort >(enableSource->getNode()))
			log::warning << L"Unsupported node type of input; Only Scalar nodes can be connected to \"Enable\" of \"traktor.render.ComputeOutput\". " << type_name(enableSource->getNode()) << L" not supported." << Endl;
	}

	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::propagateConstantExternalValues() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();

	RefArray< External > externalNodes;
	shaderGraph->findNodesOf< External >(externalNodes);
	for (auto externalNode : externalNodes)
	{
		for (auto inputPin : externalNode->getInputPins())
		{
			if (!inputPin->isOptional() || !externalNode->haveValue(inputPin->getName()))
				continue;

			Edge* edge = shaderGraph->findEdge(inputPin);
			if (!edge)
				continue;

			const Scalar* scalarNode = dynamic_type_cast< const Scalar* >(edge->getSource()->getNode());
			if (!scalarNode)
				continue;

			externalNode->setValue(inputPin->getName(), scalarNode->get());
			shaderGraph->removeEdge(edge);
		}
	}
	return shaderGraph;
}

	}
}
