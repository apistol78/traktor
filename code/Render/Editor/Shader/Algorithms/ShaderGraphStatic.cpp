/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
#include "Render/Editor/Shader/Algorithms/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphStatic.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphTypeEvaluator.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphTypePropagation.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphValidator.h"

namespace traktor::render
{
	namespace
	{

#if defined(_DEBUG)
#	define T_VALIDATE_SHADERGRAPH(sg) T_FATAL_ASSERT(ShaderGraphValidator(sg).validateIntegrity())
#else
#	define T_VALIDATE_SHADERGRAPH(sg)
#endif

struct ConstantFoldingVisitor
{
	Ref< ShaderGraph > m_shaderGraph;
	const SmallMap< const OutputPin*, Constant >& m_outputConstants;

	explicit ConstantFoldingVisitor(ShaderGraph* shaderGraph, const SmallMap< const OutputPin*, Constant >& outputConstants)
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
	m_shaderGraphId = shaderGraphId;
}

Ref< ShaderGraph > ShaderGraphStatic::getPlatformPermutation(const std::wstring& platform) const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	for (const auto node : shaderGraph->findNodesOf< Platform >())
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

		RefArray< Edge > destinationEdges = shaderGraph->findEdges(outputPin);

		shaderGraph->removeEdge(sourceEdge);
		for (const auto destinationEdge : destinationEdges)
		{
			shaderGraph->removeEdge(destinationEdge);
			shaderGraph->addEdge(new Edge(
				sourceEdge->getSource(),
				destinationEdge->getDestination()
			));
		}

		shaderGraph->removeNode(node);
	}

	T_VALIDATE_SHADERGRAPH(shaderGraph);
	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getRendererPermutation(const std::wstring& renderer) const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	for (const auto node : shaderGraph->findNodesOf< Renderer >())
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

		RefArray< Edge > destinationEdges = shaderGraph->findEdges(outputPin);

		shaderGraph->removeEdge(sourceEdge);
		for (const auto destinationEdge : destinationEdges)
		{
			shaderGraph->removeEdge(destinationEdge);
			shaderGraph->addEdge(new Edge(
				sourceEdge->getSource(),
				destinationEdge->getDestination()
			));
		}

		shaderGraph->removeNode(node);
	}

	T_VALIDATE_SHADERGRAPH(shaderGraph);
	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getConnectedPermutation() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	RefArray< Connected > connectedNodes = shaderGraph->findNodesOf< Connected >();
	if (connectedNodes.empty())
		return shaderGraph;

	for (const auto connectedNode : connectedNodes)
	{
		const InputPin* inputPin = connectedNode->findInputPin(L"Input");
		T_ASSERT(inputPin);

		const bool inputConnected = (bool)(shaderGraph->findEdge(inputPin) != nullptr);

		inputPin = connectedNode->findInputPin(inputConnected ? L"True" : L"False");
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

		const OutputPin* outputPin = connectedNode->findOutputPin(L"Output");
		T_ASSERT(outputPin);

		for (const auto destinationEdge : shaderGraph->findEdges(outputPin))
		{
			shaderGraph->removeEdge(destinationEdge);
			shaderGraph->addEdge(new Edge(
				sourceOutputPin,
				destinationEdge->getDestination()
			));
		}

		shaderGraph->removeNode(connectedNode);
	}

	T_VALIDATE_SHADERGRAPH(shaderGraph);
	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getTypePermutation() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	RefArray< Type > nodes = shaderGraph->findNodesOf< Type >();
	if (nodes.empty())
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

		RefArray< Edge > destinationEdges = shaderGraph->findEdges(outputPin);

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

	T_VALIDATE_SHADERGRAPH(shaderGraph);
	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getSwizzledPermutation() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	ShaderGraphTypePropagation typePropagation(shaderGraph, m_shaderGraphId);
	if (!typePropagation.valid())
		return nullptr;

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

	T_VALIDATE_SHADERGRAPH(shaderGraph);
	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getConstantFolded() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	SmallMap< const OutputPin*, Constant > outputConstants;
	AlignedVector< Constant > inputConstants;
	AlignedVector< const OutputPin* > inputOutputPins;

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	ShaderGraphTypePropagation typePropagation(shaderGraph, m_shaderGraphId);
	if (!typePropagation.valid())
		return nullptr;

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

			const int32_t inputPinCount = node->getInputPinCount();
			const int32_t outputPinCount = node->getOutputPinCount();

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
					// T_FATAL_ASSERT(outputPin->getNode() != node);
					inputOutputPins[j] = outputPin;

					const PinType inputPinType = typePropagation.evaluate(inputPin);
					if (isPinTypeScalar(inputPinType))
					{
						auto it = outputConstants.find(outputPin);
						T_FATAL_ASSERT (it != outputConstants.end());

						inputConstants[j] = it->second.cast(inputPinType);
					}
				}
			}

			// Evaluate result of all output pins.
			for (int32_t j = 0; j < outputPinCount; ++j)
			{
				const OutputPin* outputPin = node->getOutputPin(j);
				T_ASSERT(outputPin);

				// Don't evaluate output if it's not connected to anything.
				if (shaderGraph->getDestinationCount(outputPin) <= 0)
					continue;

				const PinType outputPinType = typePropagation.evaluate(outputPin);

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
						T_FATAL_ASSERT(foldOutputPin != outputPin);
						const PinType foldOutputPinType = typePropagation.evaluate(foldOutputPin);

						Ref< Swizzle > swizzleNode;
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

						for (auto edge : shaderGraph->findEdges(outputPin))
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
		if (traits != nullptr && traits->isRoot(shaderGraph, node))
			roots.push_back(node);
	}

	// Traverse copy shader graph; replace inputs if constant.
	ConstantFoldingVisitor visitor(new ShaderGraph(), outputConstants);
	GraphTraverse(shaderGraph, roots).preorder(visitor);

	T_VALIDATE_SHADERGRAPH(visitor.m_shaderGraph);
	return visitor.m_shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::cleanupRedundantSwizzles() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	RefArray< Swizzle > swizzleNodes = shaderGraph->findNodesOf< Swizzle >();
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
		const std::wstring swizzleRight = swizzleRightNode->get();
		const std::wstring swizzleLeft = swizzleLeftNode->get();

		const int32_t swizzleRightWidth = (int32_t)swizzleRight.length();
		const int32_t swizzleLeftWidth = (int32_t)swizzleLeft.length();

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

		T_VALIDATE_SHADERGRAPH(shaderGraph);

		// Restart iteration as it's possible we have rewired to another swizzler.
		i = swizzleNodes.begin();
	}

	T_VALIDATE_SHADERGRAPH(shaderGraph);
	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getStateResolved() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	// Pixel output state.
	for (const auto pixelOutputNode : shaderGraph->findNodesOf< PixelOutput >())
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
		resolvedPixelOutput->setPrecisionHint(state->getPrecisionHint());

		shaderGraph->removeEdge(edge);
		shaderGraph->removeNode(state);
		shaderGraph->replace(pixelOutputNode, resolvedPixelOutput);

		T_ASSERT(ShaderGraphValidator(shaderGraph).validateIntegrity());
	}

	// Sampler state.
	for (const auto samplerNode : shaderGraph->findNodesOf< Sampler >())
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

	T_VALIDATE_SHADERGRAPH(shaderGraph);
	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getBundleResolved() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	RefArray< BundleSplit > splitNodes = shaderGraph->findNodesOf< BundleSplit >();
	RefArray< BundleUnite > uniteNodes = shaderGraph->findNodesOf< BundleUnite >();

	RefArray< BundleSplit > workSplitNodes = splitNodes;
	RefArray< BundleSplit > retrySplitNodes;

	do 
	{
		for (const auto splitNode : workSplitNodes)
		{
			const OutputPin* sourcePin = shaderGraph->findSourcePin(splitNode->getInputPin(0));
			if (!sourcePin)
			{
				log::error << L"No bundle connected to split node " << splitNode->getId().format() << L"." << Endl;
				return nullptr;
			}
		
			BundleUnite* uniteNode = dynamic_type_cast< BundleUnite* >(sourcePin->getNode());
			if (!uniteNode)
			{
				if (is_a< BundleSplit >(sourcePin->getNode()))
				{
					// We must resolve source split first so we retry this node later.
					retrySplitNodes.push_back(splitNode);
					continue;
				}
				else
				{
					log::error << L"Incorrect input connected to split node, must be a bundle (is \"" << type_name(sourcePin->getNode()) << L"\")." << Endl;
					return nullptr;
				}
			}

			const int32_t outputPinCount = splitNode->getOutputPinCount();
			for (int32_t i = 0; i < outputPinCount; ++i)
			{
				const OutputPin* outputPin = splitNode->getOutputPin(i);

				const auto destinationPins = shaderGraph->findDestinationPins(outputPin);
				if (destinationPins.empty())
					continue;

				const OutputPin* sourcePin = nullptr;

				BundleUnite* uniteNodeIt = uniteNode;
				while (uniteNodeIt != nullptr)
				{
					// Get source pin connected to current unite node.
					const InputPin* inputPin = uniteNodeIt->findInputPin(outputPin->getName());
					if (inputPin)
					{
						if ((sourcePin = shaderGraph->findSourcePin(inputPin)) != nullptr)
							break;
					}

					// No such pin or not connected to a source; walk to next unite node.
					const InputPin* parentBundleInputPin = uniteNodeIt->getInputPin(0);
					T_FATAL_ASSERT(parentBundleInputPin != nullptr);

					const OutputPin* parentBundleSourcePin = shaderGraph->findSourcePin(parentBundleInputPin);
					if (parentBundleSourcePin)
					{
						uniteNodeIt = dynamic_type_cast< BundleUnite* >(parentBundleSourcePin->getNode());
						if (uniteNodeIt == nullptr)
						{
							log::error << L"Input \"Input\" into a bundle unite node must be a bundle itself." << Endl;
							return nullptr;
						}
					}
					else
						uniteNodeIt = nullptr;
				}

				for (auto edge : shaderGraph->findEdges(outputPin))
					shaderGraph->removeEdge(edge);

				if (sourcePin)
				{
					for (auto destinationPin : destinationPins)
						shaderGraph->addEdge(new Edge(sourcePin, destinationPin));
				}
			}
		}

		// Swap set with retry nodes.
		workSplitNodes.swap(retrySplitNodes);
		retrySplitNodes.resize(0);
	}
	while (!workSplitNodes.empty());

	// Remove resolved bundle splits.
	for (const auto splitNode : splitNodes)
		shaderGraph->removeNode(splitNode);

	// Remove resolved bundle unites.
	for (const auto uniteNode : uniteNodes)
		shaderGraph->removeNode(uniteNode);

	T_VALIDATE_SHADERGRAPH(shaderGraph);
	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getVariableResolved() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	// Get all variable nodes from shader graph.
	const RefArray< Variable > variableNodes = shaderGraph->findNodesOf< Variable >();

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
		shaderGraph->removeNode(variableNode);

	T_VALIDATE_SHADERGRAPH(shaderGraph);
	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::removeDisabledOutputs() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	for (const auto pixelOutputNode : shaderGraph->findNodesOf< PixelOutput >())
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
				shaderGraph->removeNode(pixelOutputNode);
			}
		}
		else // if (!is_a< InputPort >(enableSource->getNode()))
			log::warning << L"Unsupported node type of input; Only Scalar nodes can be connected to \"Enable\" of \"traktor.render.PixelOutput\". " << type_name(enableSource->getNode()) << L" not supported." << Endl;
	}

	for (const auto computeOutputNode : shaderGraph->findNodesOf< ComputeOutput >())
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
				shaderGraph->removeNode(computeOutputNode);
			}
		}
		else // if (!is_a< InputPort >(enableSource->getNode()))
			log::warning << L"Unsupported node type of input; Only Scalar nodes can be connected to \"Enable\" of \"traktor.render.ComputeOutput\". " << type_name(enableSource->getNode()) << L" not supported." << Endl;
	}

	T_VALIDATE_SHADERGRAPH(shaderGraph);
	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::propagateConstantExternalValues() const
{
	T_IMMUTABLE_CHECK(m_shaderGraph);

	Ref< ShaderGraph > shaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();
	T_VALIDATE_SHADERGRAPH(shaderGraph);

	for (auto externalNode : shaderGraph->findNodesOf< External >())
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

	T_VALIDATE_SHADERGRAPH(shaderGraph);
	return shaderGraph;
}

}
