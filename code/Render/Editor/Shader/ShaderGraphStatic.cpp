#include "Core/Containers/AlignedVector.h"
#include "Core/Log/Log.h"
#include "Render/Shader/Edge.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"
#include "Render/Editor/Shader/ShaderGraphTypeEvaluator.h"
#include "Render/Editor/Shader/ShaderGraphUtilities.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct OutputWidth
{
	PinType type;
	int32_t count;	//<! Number of destinations until width is determined; when reached zero we can be sure width is determined.
	bool parent;
};

struct ConstantFoldingVisitor
{
	Ref< ShaderGraph > m_shaderGraph;
	std::map< const OutputPin*, Constant > m_outputConstants;

	bool operator () (Node* node)
	{
		m_shaderGraph->addNode(node);
		return true;
	}

	bool operator () (Edge* edge)
	{
		const OutputPin* source = edge->getSource();

		std::map< const OutputPin*, Constant >::const_iterator it = m_outputConstants.find(source);
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
	m_shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches();
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
		else if (inputType == PntTexture)
		{
			inputPin = (*i)->findInputPin(L"Texture");
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

	std::map< const OutputPin*, OutputWidth > outputWidths;
	std::map< const InputPin*, PinType > inputWidths;
	RefArray< Node > parents;

	// Initialize "widths" of all outputs.
	for (RefArray< Node >::const_iterator i = shaderGraph->getNodes().begin(); i != shaderGraph->getNodes().end(); ++i)
	{
		uint32_t outputPinCount = (*i)->getOutputPinCount();
		for (uint32_t j = 0; j < outputPinCount; ++j)
		{
			const OutputPin* outputPin = (*i)->getOutputPin(j);
			outputWidths[outputPin].type = PntVoid;
			outputWidths[outputPin].count = shaderGraph->getDestinationCount(outputPin);
			outputWidths[outputPin].parent = false;
		}
	}

	// Find initial set of parents.
	shaderGraph->findNodesOf(type_of< VertexOutput >(), parents);
	shaderGraph->findNodesOf(type_of< PixelOutput >(), parents);

	// Iterate until all "widths" are determined.
	while (!parents.empty())
	{
		for (RefArray< Node >::iterator i = parents.begin(); i != parents.end(); ++i)
		{
			const INodeTraits* nodeTraits = findNodeTraits(*i);
			T_ASSERT (nodeTraits);

			uint32_t inputPinCount = (*i)->getInputPinCount();
			uint32_t outputPinCount = (*i)->getOutputPinCount();
			
			std::vector< PinType > outputPinTypes(outputPinCount);
			for (uint32_t j = 0; j < outputPinCount; ++j)
			{
				const OutputPin* outputPin = (*i)->getOutputPin(j);
				outputPinTypes[j] = outputWidths[outputPin].type;
			}
			
			for (uint32_t j = 0; j < inputPinCount; ++j)
			{
				const InputPin* inputPin = (*i)->getInputPin(j);
				inputWidths[inputPin] = nodeTraits->getInputPinType(
					shaderGraph,
					*i,
					inputPin,
					outputPinCount > 0 ? &outputPinTypes[0] : 0
				);
			}

			for (uint32_t j = 0; j < inputPinCount; ++j)
			{
				const InputPin* inputPin = (*i)->getInputPin(j);
				const OutputPin* outputPin = shaderGraph->findSourcePin(inputPin);

				outputWidths[outputPin].type = std::max< PinType >(outputWidths[outputPin].type, inputWidths[inputPin]);
				outputWidths[outputPin].count--;
			}
		}

		parents.resize(0);
		for (std::map< const OutputPin*, OutputWidth >::iterator i = outputWidths.begin(); i != outputWidths.end(); ++i)
		{
			if (i->second.count == 0 && i->second.parent == false)
			{
				parents.push_back(i->first->getNode());
				i->second.parent = true;
			}
		}
	}

	// Feed forward; determine output types from input types; some nodes have
	// fixed output types and we need to respect that.
	for (RefArray< Node >::const_iterator i = shaderGraph->getNodes().begin(); i != shaderGraph->getNodes().end(); ++i)
	{
		const INodeTraits* nodeTraits = findNodeTraits(*i);
		T_ASSERT (nodeTraits);

		uint32_t inputPinCount = (*i)->getInputPinCount();
		uint32_t outputPinCount = (*i)->getOutputPinCount();

		std::vector< PinType > inputPinTypes(inputPinCount);
		for (uint32_t j = 0; j < inputPinCount; ++j)
		{
			const InputPin* inputPin = (*i)->getInputPin(j);
			inputPinTypes[j] = inputWidths[inputPin];
		}

		for (uint32_t j = 0; j < outputPinCount; ++j)
		{
			const OutputPin* outputPin = (*i)->getOutputPin(j);
			outputWidths[outputPin].type = nodeTraits->getOutputPinType(
				*i,
				outputPin,
				inputPinCount > 0 ? &inputPinTypes[0] : 0
			);
		}
	}

	// Each output are now assigned a required output width.
	// Replace all edges with a "o--[swizzle]--o" pattern.
	const wchar_t* c_swizzles[] = { L"x", L"xy", L"xyz", L"xyzw" };

	RefArray< Edge > edges = shaderGraph->getEdges();
	for (RefArray< Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
	{
		const OutputPin* sourcePin = (*i)->getSource();
		const InputPin* destinationPin = (*i)->getDestination(); 

		PinType sourceType = outputWidths[sourcePin].type;
		PinType destinationType = inputWidths[destinationPin];

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
	std::map< const OutputPin*, Constant > outputConstants;
	AlignedVector< Constant > inputConstants;
	AlignedVector< Constant > inputConstantsCast;

	// Repeat folding nodes until there is no more to fold.
	RefArray< Node > nodes = m_shaderGraph->getNodes();
	for (;;)
	{
		uint32_t constantQualifiedCount = 0;
		for (RefArray< Node >::iterator i = nodes.begin(); i != nodes.end(); )
		{
			int32_t inputPinCount = (*i)->getInputPinCount();
			inputConstants.resize(inputPinCount);

			// Check if all inputs is available with constant inputs.
			int32_t constantInputCount = 0;
			for (int32_t j = 0; j < inputPinCount; ++j)
			{
				const InputPin* inputPin = (*i)->getInputPin(j);
				T_ASSERT (inputPin);

				const OutputPin* outputPin = m_shaderGraph->findSourcePin(inputPin);
				if (!outputPin)
					continue;

				std::map< const OutputPin*, Constant >::const_iterator it = outputConstants.find(outputPin);
				if (it != outputConstants.end())
				{
					inputConstants[j] = it->second;
					++constantInputCount;
				}
			}

			if (constantInputCount == inputPinCount)
			{
				// Full constant input; should be able to calculate output.
				const INodeTraits* nodeTraits = findNodeTraits(*i);
				if (nodeTraits)
				{
					int32_t outputPinCount = (*i)->getOutputPinCount();

					// Prepare input pin types from found constants.
					PinType inputPinTypes[16];
					for (int32_t j = 0; j < inputPinCount; ++j)
						inputPinTypes[j] = inputConstants[j].getType();

					// Determine output pin types from given set of inputs.
					PinType outputPinTypes[16];
					for (int32_t j = 0; j < outputPinCount; ++j)
					{
						const OutputPin* outputPin = (*i)->getOutputPin(j);
						T_ASSERT (outputPin);

						outputPinTypes[j] = nodeTraits->getOutputPinType(*i, outputPin, inputPinTypes);
					}

					inputConstantsCast.resize(inputPinCount);
					for (int32_t j = 0; j < outputPinCount; ++j)
					{
						const OutputPin* outputPin = (*i)->getOutputPin(j);
						T_ASSERT (outputPin);

						// Cast input constants to required types.
						for (int32_t k = 0; k < inputPinCount; ++k)
						{
							const InputPin* inputPin = (*i)->getInputPin(k);
							T_ASSERT (inputPin);

							PinType inputPinType = nodeTraits->getInputPinType(
								m_shaderGraph,
								*i,
								inputPin,
								outputPinTypes
							);

							inputConstantsCast[k] = inputConstants[k].cast(inputPinType);
						}
					
						// Evaluate output constant.
						Constant outputConstant(outputPinTypes[j]);
						if (nodeTraits->evaluateFull(
							m_shaderGraph,
							*i,
							outputPin,
							inputPinCount > 0 ? &inputConstantsCast[0] : 0,
							outputConstant
						))
						{
							outputConstants[outputPin] = outputConstant;
						}
					}
				}

				++constantQualifiedCount;
				i = nodes.erase(i);
			}
			else
			{
				// Partial constant input; need to reiterate.
				++i;
			}
		}

		if (!constantQualifiedCount)
		{
			// No more constant nodes found; see if we can collapse partially constant nodes
			// such as multiply with zero etc.

			uint32_t partialQualifiedCount = 0;
			for (RefArray< Node >::iterator i = nodes.begin(); i != nodes.end(); )
			{
				// Partial nodes must have at least two inputs (at least one being constant).
				int32_t inputPinCount = (*i)->getInputPinCount();
				if (inputPinCount < 2)
				{
					++i;
					continue;
				}

				// Get available constants.
				inputConstants.resize(inputPinCount);

				int32_t constantInputCount = 0;
				for (int32_t j = 0; j < inputPinCount; ++j)
				{
					const InputPin* inputPin = (*i)->getInputPin(j);
					T_ASSERT (inputPin);

					const OutputPin* outputPin = m_shaderGraph->findSourcePin(inputPin);
					if (!outputPin)
						continue;

					std::map< const OutputPin*, Constant >::const_iterator it = outputConstants.find(outputPin);
					if (it != outputConstants.end())
					{
						inputConstants[j] = it->second;
						++constantInputCount;
					}
					else
						inputConstants[j] = Constant(PntVoid);	// All inputs which isn't constant is void.
				}

				// If no inputs are constant then we cannot evaluate a conclusive result.
				if (constantInputCount <= 0)
				{
					++i;
					continue;
				}

				const INodeTraits* nodeTraits = findNodeTraits(*i);
				if (!nodeTraits)
				{
					++i;
					continue;
				}

				// Evaluate output constant from partial constant input set.
				int32_t outputPinCount = (*i)->getOutputPinCount();
				int32_t outputConstantEvaluated = 0;
				for (int32_t j = 0; j < outputPinCount; ++j)
				{
					const OutputPin* outputPin = (*i)->getOutputPin(j);
					T_ASSERT (outputPin);
						
					Constant outputConstant;
					if (nodeTraits->evaluatePartial(
						m_shaderGraph,
						*i,
						outputPin,
						&inputConstants[0],
						outputConstant
					))
					{
						outputConstants[outputPin] = outputConstant;
						++partialQualifiedCount;
						++outputConstantEvaluated;
					}
				}

				if (outputConstantEvaluated >= outputPinCount)
					i = nodes.erase(i);
				else
					++i;

				//for (int32_t j = 0; j < inputPinCount; ++j)
				//{
				//	const InputPin* inputPin = (*i)->getInputPin(j);
				//	T_ASSERT (inputPin);

				//	const OutputPin* outputPin = m_shaderGraph->findSourcePin(inputPin);
				//	if (!outputPin)
				//		continue;

				//	std::map< const OutputPin*, Constant >::const_iterator it = outputConstants.find(outputPin);
				//	if (it != outputConstants.end() && it->second.isZero())
				//	{
				//		inputZero = true;
				//		break;
				//	}
				//}

				//if (inputZero)
				//{
				//	outputConstants[(*i)->getOutputPin(0)] = Constant(0.0f);
				//	++partialQualifiedCount;

				//	i = nodes.erase(i);
				//}
				//else
				//	++i;
			}

			if (!partialQualifiedCount)
				break;
		}
	}

	// Collect root nodes; assume all nodes with no output pins to be roots.
	RefArray< Node > roots;
	nodes = m_shaderGraph->getNodes();
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if ((*i)->getOutputPinCount() <= 0)
			roots.push_back(*i);
	}

	// Traverse copy shader graph; replace inputs if constant.
	ConstantFoldingVisitor visitor;
	visitor.m_shaderGraph = new ShaderGraph();
	visitor.m_outputConstants = outputConstants;
	shaderGraphTraverse(m_shaderGraph, roots, visitor);

	return visitor.m_shaderGraph;
}

	}
}
