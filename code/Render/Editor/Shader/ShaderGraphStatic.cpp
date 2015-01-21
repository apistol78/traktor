#include <cctype>
#include "Core/Containers/AlignedVector.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Render/Shader/Edge.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/ShaderGraphTraverse.h"
#include "Render/Editor/Shader/INodeTraits.h"
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
			case PntScalar1:
				node = new Scalar(it->second.x());
				break;

			case PntScalar2:
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

			case PntScalar3:
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

			case PntScalar4:
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

struct ReadVariablePred
{
	const ShaderGraph* m_shaderGraph;
	std::wstring m_name;

	ReadVariablePred(const ShaderGraph* shaderGraph, const std::wstring& name)
	:	m_shaderGraph(shaderGraph)
	,	m_name(name)
	{
	}

	bool operator () (const Variable* node) const
	{
		if (node->getName() == m_name)
		{
			// Matching names, is this variable written to?
			if (m_shaderGraph->findSourcePin(node->getInputPin(0)))
				return true;
		}
		return false;
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
	Ref< ShaderGraph > shaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();

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

	return shaderGraph;
}

Ref< ShaderGraph > ShaderGraphStatic::getTypePermutation() const
{
	Ref< ShaderGraph > shaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();

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
		else if (inputType == PntVoid)
		{
			inputPin = (*i)->findInputPin(L"Default");
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
	Ref< ShaderGraph > shaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();
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
	std::vector< const OutputPin* > inputOutputPins;

	Ref< ShaderGraph > shaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();
	T_ASSERT (shaderGraph);
	T_ASSERT (ShaderGraphValidator(shaderGraph).validateIntegrity());

	ShaderGraphTypePropagation typePropagation(shaderGraph);
	const RefArray< Node >& nodes = shaderGraph->getNodes();

	// Setup map of all output pin "constants"; each such constant has proper width and
	// are initially set to be completely variant.
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		const INodeTraits* nodeTraits = INodeTraits::find(*i);
		T_FATAL_ASSERT (nodeTraits);

		int32_t outputPinCount = (*i)->getOutputPinCount();
		for (int32_t j = 0; j < outputPinCount; ++j)
		{
			const OutputPin* outputPin = (*i)->getOutputPin(j);
			T_ASSERT (outputPin);

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

		for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			const INodeTraits* nodeTraits = INodeTraits::find(*i);
			T_ASSERT (nodeTraits);

			int32_t inputPinCount = (*i)->getInputPinCount();

			// Get set of input constants and source pins.
			inputConstants.resize(inputPinCount);
			inputOutputPins.resize(inputPinCount);
			for (int32_t j = 0; j < inputPinCount; ++j)
			{
				const InputPin* inputPin = (*i)->getInputPin(j);
				T_ASSERT (inputPin);

				inputConstants[j] = Constant();
				inputOutputPins[j] = 0;

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
			int32_t outputPinCount = (*i)->getOutputPinCount();
			for (int32_t j = 0; j < outputPinCount; ++j)
			{
				const OutputPin* outputPin = (*i)->getOutputPin(j);
				T_ASSERT (outputPin);

				// Don't evaluate output if it's not connected to anything.
				if (shaderGraph->getDestinationCount(outputPin) <= 0)
					continue;

				PinType outputPinType = typePropagation.evaluate(outputPin);
				T_ASSERT (outputPinType != PntVoid);

				// First attempt to evaluate re-wiring to circumvent this node entirely.
				if (inputPinCount > 0)
				{
					const OutputPin* foldOutputPin = 0;
					if (nodeTraits->evaluatePartial(
						shaderGraph,
						*i,
						outputPin,
						&inputOutputPins[0],
						&inputConstants[0],
						foldOutputPin
					))
					{
						Ref< Swizzle > swizzleNode;

						// In case re-wire to pin is returned null then entire graph is folded/discarded.
						if (!foldOutputPin)
							return 0;

						PinType outputPinType = typePropagation.evaluate(outputPin);
						PinType foldOutputPinType = typePropagation.evaluate(foldOutputPin);

						if (isPinTypeScalar(outputPinType) && isPinTypeScalar(foldOutputPinType))
						{
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
						}

						RefSet< Edge > edges;
						shaderGraph->findEdges(outputPin, edges);

						for (RefSet< Edge >::const_iterator it = edges.begin(); it != edges.end(); ++it)
						{
							shaderGraph->removeEdge(*it);

							Ref< Edge > foldEdge = new Edge(foldOutputPin, (*it)->getDestination());
							shaderGraph->addEdge(foldEdge);
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
					*i,
					outputPin,
					inputPinCount > 0 ? &inputConstants[0] : 0,
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

Ref< ShaderGraph > ShaderGraphStatic::getVariableResolved() const
{
	Ref< ShaderGraph > shaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();

	RefArray< Variable > variableNodes;
	shaderGraph->findNodesOf< Variable >(variableNodes);

	for (RefArray< Variable >::iterator i = variableNodes.begin(); i != variableNodes.end(); ++i)
	{
		const OutputPin* variableOutput = (*i)->getOutputPin(0);
		T_ASSERT (variableOutput);

		if (shaderGraph->getDestinationCount(variableOutput) > 0)
		{
			RefArray< Variable >::iterator j = std::find_if(variableNodes.begin(), variableNodes.end(), ReadVariablePred(shaderGraph, (*i)->getName()));
			if (j == variableNodes.end())
			{
				log::error << L"Unable to read variable \"" << (*i)->getName() << L", no such variable." << Endl;
				return 0;
			}

			const InputPin* variableInput = (*j)->getInputPin(0);
			T_ASSERT (variableInput);

			const OutputPin* sourcePin = shaderGraph->findSourcePin(variableInput);
			T_ASSERT (sourcePin);

			shaderGraph->rewire(
				variableOutput,
				sourcePin
			);
		}
	}

	return ShaderGraphOptimizer(shaderGraph).removeUnusedBranches();
}

	}
}
