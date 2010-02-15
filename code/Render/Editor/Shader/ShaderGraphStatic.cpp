#include "Render/Shader/Edge.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"
#include "Render/Editor/Shader/ShaderGraphTypeEvaluator.h"

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

	}
}
