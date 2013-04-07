#include <cctype>
#include "Core/Containers/AlignedVector.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Render/Shader/Edge.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"
#include "Render/Editor/Shader/ShaderGraphTypeEvaluator.h"
#include "Render/Editor/Shader/ShaderGraphTypePropagation.h"
#include "Render/Editor/Shader/ShaderGraphUtilities.h"
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
		if (it != m_outputConstants.end())
		{
			std::pair< int, int > position = source->getNode()->getPosition();
			Ref< Node > node;

			switch (it->second.getType())
			{
			case PntScalar1:
				node = new Scalar(it->second[0]);
				break;

			case PntScalar2:
				{
					Ref< Vector > value = new Vector(Vector4(it->second[0], it->second[1], 0.0f, 0.0f));
					Ref< Swizzle > swizzle = new Swizzle(L"xy");

					m_shaderGraph->addNode(value);
					m_shaderGraph->addEdge(new Edge(value->getOutputPin(0), swizzle->getInputPin(0)));

					value->setPosition(position);
					position.first += 32;

					node = swizzle;
				}
				break;

			case PntScalar3:
				{
					Ref< Vector > value = new Vector(Vector4(it->second[0], it->second[1], it->second[2], 0.0f));
					Ref< Swizzle > swizzle = new Swizzle(L"xyz");

					m_shaderGraph->addNode(value);
					m_shaderGraph->addEdge(new Edge(value->getOutputPin(0), swizzle->getInputPin(0)));

					value->setPosition(position);
					position.first += 32;

					node = swizzle;
				}
				break;

			case PntScalar4:
				node = new Vector(Vector4(it->second[0], it->second[1], it->second[2], it->second[3]));
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

ShaderGraphStatic::ShaderGraphStatic(const ShaderGraph* shaderGraph)
{
	T_ASSERT (ShaderGraphValidator(shaderGraph).validateIntegrity());
	m_shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches();
	T_ASSERT (ShaderGraphValidator(m_shaderGraph).validateIntegrity());
}

Ref< ShaderGraph > ShaderGraphStatic::getPlatformPermutation(const std::wstring& platform) const
{
	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	RefArray< Platform > nodes;
	shaderGraph->findNodesOf< Platform >(nodes);
	for (RefArray< Platform >::iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		const InputPin* inputPin = (*i)->findInputPin(platform);
		T_ASSERT (inputPin);

		Ref< Edge > sourceEdge = shaderGraph->findEdge(inputPin);
		if (!sourceEdge)
		{
			inputPin = (*i)->findInputPin(L"Other");
			T_ASSERT (inputPin);

			sourceEdge = shaderGraph->findEdge(inputPin);
			if (!sourceEdge)
				return 0;
		}

		const OutputPin* outputPin = (*i)->findOutputPin(L"Output");
		T_ASSERT (outputPin);

		RefSet< Edge > destinationEdges;
		shaderGraph->findEdges(outputPin, destinationEdges);

		shaderGraph->removeEdge(sourceEdge);
		for (RefSet< Edge >::const_iterator j = destinationEdges.begin(); j != destinationEdges.end(); ++j)
		{
			shaderGraph->removeEdge(*j);
			shaderGraph->addEdge(new Edge(
				sourceEdge->getSource(),
				(*j)->getDestination()
			));
		}
	}

	return ShaderGraphOptimizer(shaderGraph).removeUnusedBranches();
}

Ref< ShaderGraph > ShaderGraphStatic::getTypePermutation() const
{
	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	RefArray< Type > nodes;
	if (shaderGraph->findNodesOf< Type >(nodes) <= 0)
		return shaderGraph;

	ShaderGraphTypeEvaluator evaluator(shaderGraph);
	for (RefArray< Type >::iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		const OutputPin* outputPin = (*i)->findOutputPin(L"Output");
		T_ASSERT (outputPin);

		PinType inputType = evaluator.evaluate(*i, L"Type");

		const InputPin* inputPin = 0;

		if (isPinTypeScalar(inputType))
		{
			if (getPinTypeWidth(inputType) <= 1)
				inputPin = (*i)->findInputPin(L"Scalar");
			else
				inputPin = (*i)->findInputPin(L"Vector");
		}
		else if (inputType == PntMatrix)
		{
			inputPin = (*i)->findInputPin(L"Matrix");
		}
		else if (isPinTypeTexture(inputType))
		{
			inputPin = (*i)->findInputPin(L"Texture");
		}
		else if (isPinTypeState(inputType))
		{
			inputPin = (*i)->findInputPin(L"State");
		}

		if (!inputPin)
			return 0;

		Ref< Edge > sourceEdge = shaderGraph->findEdge(inputPin);
		if (!sourceEdge)
		{
			inputPin = (*i)->findInputPin(L"Default");
			T_ASSERT (inputPin);

			sourceEdge = shaderGraph->findEdge(inputPin);
			if (!sourceEdge)
				return 0;
		}

		RefSet< Edge > destinationEdges;
		shaderGraph->findEdges(outputPin, destinationEdges);

		shaderGraph->removeEdge(sourceEdge);
		for (RefSet< Edge >::const_iterator j = destinationEdges.begin(); j != destinationEdges.end(); ++j)
		{
			shaderGraph->removeEdge(*j);
			shaderGraph->addEdge(new Edge(
				sourceEdge->getSource(),
				(*j)->getDestination()
			));
		}
	}

	return ShaderGraphOptimizer(shaderGraph).removeUnusedBranches();
}

Ref< ShaderGraph > ShaderGraphStatic::getSwizzledPermutation() const
{
	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);

	ShaderGraphTypePropagation typePropagation(shaderGraph);

	// Each output are now assigned a required output width.
	// Replace all edges with a "o--[swizzle]--o" pattern.
	const wchar_t* c_swizzles[] = { L"x", L"xy", L"xyz", L"xyzw" };

	RefArray< Edge > edges = shaderGraph->getEdges();
	for (RefArray< Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
	{
		const OutputPin* sourcePin = (*i)->getSource();
		const InputPin* destinationPin = (*i)->getDestination(); 

		PinType sourceType = typePropagation.evaluate(sourcePin);
		PinType destinationType = typePropagation.evaluate(destinationPin);

		if (sourceType <= destinationType || !isPinTypeScalar(sourceType) || !isPinTypeScalar(destinationType))
			continue;

		// \fixme Bug in shader graph; must remove edge prior to adding new edges.
		shaderGraph->removeEdge(*i);

		const std::pair< int, int > p1 = sourcePin->getNode()->getPosition();
		const std::pair< int, int > p2 = destinationPin->getNode()->getPosition();

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
	SmallMap< const OutputPin*, Constant > outputConstants;
	AlignedVector< Constant > inputConstants;
	AlignedVector< Constant > inputConstantsCast;
	std::vector< const OutputPin* > inputOutputPins;

	Ref< ShaderGraph > shaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();
	T_ASSERT (shaderGraph);
	T_ASSERT (ShaderGraphValidator(shaderGraph).validateIntegrity());

	ShaderGraphTypePropagation typePropagation(shaderGraph);

	const RefArray< Node >& nodes = shaderGraph->getNodes();

	// Collect all constant leafs.
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if ((*i)->getInputPinCount() > 0)
			continue;

		const INodeTraits* nodeTraits = INodeTraits::find(*i);
		if (!nodeTraits)
			continue;

		int32_t outputPinCount = (*i)->getOutputPinCount();
		for (int32_t j = 0; j < outputPinCount; ++j)
		{
			const OutputPin* outputPin = (*i)->getOutputPin(j);
			T_ASSERT (outputPin);

			PinType outputPinType = typePropagation.evaluate(outputPin);
			T_ASSERT (outputPinType != PntVoid);

			Constant outputConstant(outputPinType);
			if (nodeTraits->evaluateFull(
				shaderGraph,
				*i,
				outputPin,
				0,
				outputConstant
			))
			{
				outputConstants[outputPin] = outputConstant;
			}
		}
	}

	// If we don't have any constant leafs then there
	// cannot be any further constant folding.
	if (outputConstants.empty())
		return shaderGraph;

	// Repeat folding nodes until there is no more to fold.
	for (;;)
	{
		// Fold nodes which have all of it's inputs constant.
		// Evaluate nodes which have all inputs constant; if able to calculate
		// result then we increase "qualified counter" and add result to outputConstants map.
		for (;;)
		{
			uint32_t constantQualifiedCount = 0;
			for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
			{
				int32_t inputPinCount = (*i)->getInputPinCount();
				if (inputPinCount <= 0)
					continue;

				inputConstants.resize(inputPinCount);

				// Check if all inputs is constants.
				int32_t constantInputCount = 0;
				for (int32_t j = 0; j < inputPinCount; ++j)
				{
					const InputPin* inputPin = (*i)->getInputPin(j);
					T_ASSERT (inputPin);

					const OutputPin* outputPin = shaderGraph->findSourcePin(inputPin);
					if (!outputPin)
						continue;

					PinType inputPinType = typePropagation.evaluate(inputPin);
					if (!isPinTypeScalar(inputPinType))
						continue;

					SmallMap< const OutputPin*, Constant >::const_iterator it = outputConstants.find(outputPin);
					if (it != outputConstants.end())
					{
						inputConstants[j] = it->second.cast(inputPinType);
						++constantInputCount;
					}
				}

				if (constantInputCount != inputPinCount)
					continue;

				// All inputs are constant; try to evaluate constant result for each output.
				const INodeTraits* nodeTraits = INodeTraits::find(*i);
				if (!nodeTraits)
					continue;

				int32_t outputPinCount = (*i)->getOutputPinCount();
				for (int32_t j = 0; j < outputPinCount; ++j)
				{
					const OutputPin* outputPin = (*i)->getOutputPin(j);
					T_ASSERT (outputPin);

					// Is output already classified as constant?
					if (outputConstants.find(outputPin) != outputConstants.end())
						continue;

					PinType outputPinType = typePropagation.evaluate(outputPin);
					T_ASSERT (outputPinType != PntVoid);

					// Evaluate output constant.
					Constant outputConstant(outputPinType);
					if (nodeTraits->evaluateFull(
						shaderGraph,
						*i,
						outputPin,
						inputPinCount > 0 ? &inputConstants[0] : 0,
						outputConstant
					))
					{
						outputConstants[outputPin] = outputConstant;
						++constantQualifiedCount;
					}
				}
			}

			// No more outputs can be evaluated constantly.
			if (!constantQualifiedCount)
				break;
		}

		// No more constant nodes evaluted; begin second phase of collapsing
		// partially constant nodes such as multiplication with zero etc.

		// Iterate by index as we will add nodes at tail when partially constants
		// nodes are found.
		uint32_t partialQualifiedCount = 0;
		for (uint32_t i = 0; i < nodes.size(); ++i)
		{
			const Node* node = nodes[i];
			T_ASSERT (node);

			// Partial nodes must have at least two inputs and at least one being constant.
			int32_t inputPinCount = node->getInputPinCount();
			if (inputPinCount < 2)
				continue;

			// Get available constants.
			inputOutputPins.resize(inputPinCount);
			inputConstants.resize(inputPinCount);

			int32_t constantInputCount = 0;
			for (int32_t j = 0; j < inputPinCount; ++j)
			{
				const InputPin* inputPin = node->getInputPin(j);
				T_ASSERT (inputPin);

				const OutputPin* outputPin = shaderGraph->findSourcePin(inputPin);
				if (!outputPin)
					continue;

				inputOutputPins[j] = outputPin;

				SmallMap< const OutputPin*, Constant >::const_iterator it = outputConstants.find(outputPin);
				if (it != outputConstants.end())
				{
					PinType inputPinType = typePropagation.evaluate(inputPin);
					T_ASSERT (inputPinType != PntVoid);

					inputConstants[j] = it->second.cast(inputPinType);
					++constantInputCount;
				}
				else
				{
					// All inputs which isn't constant is void.
					inputConstants[j] = Constant(PntVoid);
				}
			}

			// If no inputs are constant then we cannot evaluate a conclusive result.
			if (constantInputCount <= 0)
				continue;

			const INodeTraits* nodeTraits = INodeTraits::find(node);
			if (!nodeTraits)
				continue;

			// Evaluate output constant from partial constant input set.
			int32_t outputPinCount = node->getOutputPinCount();
			for (int32_t j = 0; j < outputPinCount; ++j)
			{
				const OutputPin* outputPin = node->getOutputPin(j);
				T_ASSERT (outputPin);

				// Ignore evaluation if output is already classified as constant.
				if (outputConstants.find(outputPin) != outputConstants.end())
					continue;

				// Don't evaluate output if it's not connected to anything.
				if (shaderGraph->getDestinationCount(outputPin) <= 0)
					continue;

				// First evaluate constant result from partial constant input set.
				Constant outputConstant;
				if (nodeTraits->evaluatePartial(
					shaderGraph,
					node,
					outputPin,
					&inputConstants[0],
					outputConstant
				))
				{
					if (outputConstant.getWidth() > 0)
					{
						outputConstants[outputPin] = outputConstant;
						++partialQualifiedCount;
						continue;
					}
				}

				// Then try to evaluate rewiring from partial constant input set.
				const OutputPin* foldOutputPin = 0;
				if (nodeTraits->evaluatePartial(
					shaderGraph,
					node,
					outputPin,
					&inputOutputPins[0],
					&inputConstants[0],
					foldOutputPin
				))
				{
					if (foldOutputPin)
					{
						PinType outputPinType = typePropagation.evaluate(outputPin);
						PinType foldOutputPinType = typePropagation.evaluate(foldOutputPin);

						if (!isPinTypeScalar(outputPinType) || !isPinTypeScalar(foldOutputPinType))
							continue;

						Ref< Swizzle > swizzleNode;
						if (outputPinType != foldOutputPinType)
						{
							// Need to add a swizzle node in order to expand into expected type.
							T_ASSERT (foldOutputPinType < outputPinType);

							if (foldOutputPinType == PntScalar1)
							{
								const wchar_t* c_scalarSwizzles[] = { L"", L"xx", L"xxx", L"xxxx" };
								swizzleNode = new Swizzle(c_scalarSwizzles[getPinTypeWidth(outputPinType) - 1]);
							}
							else if (foldOutputPinType == PntScalar2)
							{
								const wchar_t* c_tupleSwizzles[] = { L"", L"", L"xy0", L"xy00" };
								swizzleNode = new Swizzle(c_tupleSwizzles[getPinTypeWidth(outputPinType) - 1]);
							}
							else if (foldOutputPinType == PntScalar3)
							{
								const wchar_t* c_tripleSwizzles[] = { L"", L"", L"", L"xyz0" };
								swizzleNode = new Swizzle(c_tripleSwizzles[getPinTypeWidth(outputPinType) - 1]);
							}
							else
								continue;
						}

						if (swizzleNode)
						{
							Ref< Edge > edgeIn = new Edge(foldOutputPin, swizzleNode->getInputPin(0));
							
							shaderGraph->addEdge(edgeIn);
							shaderGraph->addNode(swizzleNode);

							typePropagation.set(swizzleNode->getInputPin(0), outputPinType);
							typePropagation.set(swizzleNode->getOutputPin(0), outputPinType);

							foldOutputPin = swizzleNode->getOutputPin(0);
						}

						RefSet< Edge > edges;
						shaderGraph->findEdges(outputPin, edges);

						for (RefSet< Edge >::const_iterator it = edges.begin(); it != edges.end(); ++it)
						{
							shaderGraph->removeEdge(*it);

							Ref< Edge > foldEdge = new Edge(foldOutputPin, (*it)->getDestination());
							shaderGraph->addEdge(foldEdge);
						}

						++partialQualifiedCount;
						continue;
					}
				}
			}
		}

		if (!partialQualifiedCount)
			break;
	}

	// Collect root nodes; assume all nodes with no output pins to be roots.
	RefArray< Node > roots;
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if ((*i)->getOutputPinCount() <= 0 && (*i)->getInputPinCount() > 0)
			roots.push_back(*i);
	}

	// Traverse copy shader graph; replace inputs if constant.
	ConstantFoldingVisitor visitor(new ShaderGraph(), outputConstants);
	ShaderGraphTraverse(shaderGraph, roots).preorder(visitor);

	T_ASSERT (ShaderGraphValidator(visitor.m_shaderGraph).validateIntegrity());
	return visitor.m_shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::cleanupRedundantSwizzles() const
{
	Ref< ShaderGraph > shaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();

	RefArray< Swizzle > swizzleNodes;
	shaderGraph->findNodesOf< Swizzle >(swizzleNodes);

	for (RefArray< Swizzle >::iterator i = swizzleNodes.begin(); i != swizzleNodes.end(); )
	{
		Swizzle* swizzleRightNode = *i;
		T_ASSERT (swizzleRightNode);

		const InputPin* swizzleInput = swizzleRightNode->getInputPin(0);
		T_ASSERT (swizzleInput);

		const OutputPin* swizzleOutput = swizzleRightNode->getOutputPin(0);
		T_ASSERT (swizzleOutput);

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
		shaderGraph->addEdge(new Edge(
			shaderGraph->findSourcePin(swizzleLeftNode->getInputPin(0)),
			swizzleRightNode->getInputPin(0)
		));

		T_ASSERT (ShaderGraphValidator(shaderGraph).validateIntegrity());

		// Restart iteration as it's possible we have rewired to another swizzler.
		i = swizzleNodes.begin();
	}

	return ShaderGraphOptimizer(shaderGraph).removeUnusedBranches();
}

Ref< ShaderGraph > ShaderGraphStatic::getStateResolved() const
{
	Ref< ShaderGraph > shaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();

	RefArray< PixelOutput > pixelOutputNodes;
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputNodes);

	for (RefArray< PixelOutput >::iterator i = pixelOutputNodes.begin(); i != pixelOutputNodes.end(); ++i)
	{
		Ref< Edge > edge = shaderGraph->findEdge((*i)->getInputPin(4));
		if (!edge)
			continue;

		Ref< State > state = dynamic_type_cast< State* >(edge->getSource()->getNode());
		if (!state)
			continue;

		(*i)->setPriority(state->getPriority());
		(*i)->setRenderState(state->getRenderState());

		shaderGraph->removeEdge(edge);
		shaderGraph->removeNode(state);

		T_ASSERT (ShaderGraphValidator(shaderGraph).validateIntegrity());
	}

	return shaderGraph;
}

	}
}
