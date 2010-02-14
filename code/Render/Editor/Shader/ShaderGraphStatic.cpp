#include <cctype>
#include "Render/Shader/Edge.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
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
	int32_t width;
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

		if (inputType.getClass() == PtcScalar)
		{
			if (inputType.getWidth() <= 1)
				inputPin = (*i)->findInputPin(L"Scalar");
			else
				inputPin = (*i)->findInputPin(L"Vector");
		}
		else if (inputType.getClass() == PtcMatrix)
		{
			inputPin = (*i)->findInputPin(L"Matrix");
		}
		else if (inputType.getClass() == PtcTexture)
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
	std::map< const InputPin*, int32_t > inputWidths;
	RefArray< Node > parents;

	// Initialize "widths" of all outputs.
	for (RefArray< Node >::const_iterator i = shaderGraph->getNodes().begin(); i != shaderGraph->getNodes().end(); ++i)
	{
		uint32_t outputPinCount = (*i)->getOutputPinCount();
		for (uint32_t j = 0; j < outputPinCount; ++j)
		{
			const OutputPin* outputPin = (*i)->getOutputPin(j);
			outputWidths[outputPin].width = 0;
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
			// Determine input widths.
			if (const Lerp* lerpNode = dynamic_type_cast< const Lerp* >(*i))
			{
				const OutputPin* outputPin = lerpNode->getOutputPin(0);
				T_ASSERT (outputWidths[outputPin].count == 0);

				int32_t outputWidth = outputWidths[outputPin].width;

				inputWidths[lerpNode->findInputPin(L"Input1")] = outputWidth;
				inputWidths[lerpNode->findInputPin(L"Input2")] = outputWidth;
				inputWidths[lerpNode->findInputPin(L"Blend")] = 1;
			}
			else if (const Swizzle* swizzleNode = dynamic_type_cast< const Swizzle* >(*i))
			{
				const std::wstring& pattern = swizzleNode->get();

				int32_t swizzleWidth = 0;
				for (std::wstring::const_iterator k = pattern.begin(); k != pattern.end(); ++k)
				{
					switch (std::tolower(*k))
					{
					case 'x':
						swizzleWidth = std::max< int32_t >(swizzleWidth, 1);
						break;
					case 'y':
						swizzleWidth = std::max< int32_t >(swizzleWidth, 2);
						break;
					case 'z':
						swizzleWidth = std::max< int32_t >(swizzleWidth, 3);
						break;
					case 'w':
						swizzleWidth = std::max< int32_t >(swizzleWidth, 4);
						break;
					}
				}

				inputWidths[swizzleNode->findInputPin(L"Input")] = swizzleWidth;
			}
			else if (is_a< VertexOutput >(*i) || is_a< PixelOutput >(*i))
			{
				// Output nodes require full width.
				inputWidths[(*i)->findInputPin(L"Input")] = 4;
			}
			else if (is_a< Sum >(*i) || is_a< Iterate >(*i))
			{
				const OutputPin* outputPin = (*i)->findOutputPin(L"Output");
				T_ASSERT (outputWidths[outputPin].count == 0);

				inputWidths[(*i)->findInputPin(L"Input")] = outputWidths[outputPin].width;
			}
			else if (is_a< IndexedUniform >(*i))
			{
				inputWidths[(*i)->findInputPin(L"Index")] = 1;
			}
			else if (is_a< Length >(*i))
			{
				inputWidths[(*i)->findInputPin(L"Input")] = 4;
			}
			else if (is_a< Matrix >(*i))
			{
				inputWidths[(*i)->findInputPin(L"XAxis")] = 4;
				inputWidths[(*i)->findInputPin(L"YAxis")] = 4;
				inputWidths[(*i)->findInputPin(L"ZAxis")] = 4;
				inputWidths[(*i)->findInputPin(L"Translate")] = 4;
			}
			else if (is_a< Polynomial >(*i))
			{
				inputWidths[(*i)->findInputPin(L"X")] = 1;
				inputWidths[(*i)->findInputPin(L"Coefficients")] = 4;
			}
			else if (is_a< Sampler >(*i))
			{
				inputWidths[(*i)->findInputPin(L"TexCoord")] = 4;
			}
			else if (is_a< MixIn >(*i))
			{
				inputWidths[(*i)->findInputPin(L"X")] = 1;
				inputWidths[(*i)->findInputPin(L"Y")] = 1;
				inputWidths[(*i)->findInputPin(L"Z")] = 1;
				inputWidths[(*i)->findInputPin(L"W")] = 1;
			}
			else if (is_a< MixOut >(*i))
			{
				inputWidths[(*i)->findInputPin(L"Input")] = 4;
			}
			else if (is_a< Transform >(*i))
			{
				inputWidths[(*i)->findInputPin(L"Input")] = 4;
			}
			else if (is_a< Dot >(*i))
			{
				inputWidths[(*i)->findInputPin(L"Input1")] = 4;
				inputWidths[(*i)->findInputPin(L"Input2")] = 4;
			}
			else
			{
				uint32_t outputPinCount = (*i)->getOutputPinCount();

				// Determine maximum width from all our outputs.
				int32_t inputWidth = 0;
				for (uint32_t j = 0; j < outputPinCount; ++j)
				{
					const OutputPin* outputPin = (*i)->getOutputPin(j);
					T_ASSERT (outputWidths[outputPin].count == 0);

					inputWidth = std::max< int32_t >(
						inputWidth,
						outputWidths[outputPin].width
					);
				}

				uint32_t inputPinCount = (*i)->getInputPinCount();
				for (uint32_t j = 0; j < inputPinCount; ++j)
					inputWidths[(*i)->getInputPin(j)] = inputWidth;
			}

			// Assign width to child outputs.
			uint32_t inputPinCount = (*i)->getInputPinCount();
			for (uint32_t j = 0; j < inputPinCount; ++j)
			{
				const InputPin* inputPin = (*i)->getInputPin(j);
				const OutputPin* outputPin = shaderGraph->findSourcePin(inputPin);

				int32_t inputWidth = inputWidths[inputPin];

				outputWidths[outputPin].width = std::max< int32_t >(outputWidths[outputPin].width, inputWidth);
				outputWidths[outputPin].count--;
			}
		}

		// Determine new set of parents; all nodes which have been fully determined and not already
		// been a parent is the new set.
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

	// Fix output width of constants.
	for (RefArray< Node >::const_iterator i = shaderGraph->getNodes().begin(); i != shaderGraph->getNodes().end(); ++i)
	{
		uint32_t inputPinCount = (*i)->getInputPinCount();
		uint32_t outputPinCount = (*i)->getOutputPinCount();

		if (inputPinCount != 0 && !is_a< IndexedUniform >(*i) && !is_a< Sampler >(*i))	// \fixme How to determine?
			continue;

		ShaderGraphTypeEvaluator typeEvaluator(shaderGraph);
		for (uint32_t j = 0; j < outputPinCount; ++j)
		{
			const OutputPin* outputPin = (*i)->getOutputPin(j);
			
			PinType pinType = typeEvaluator.evaluate(outputPin);
			if (pinType.getClass() == PtcScalar)
				outputWidths[outputPin].width = pinType.getWidth();
			else
				outputWidths[outputPin].width = 0;
		}
	}

	// Each output are now assigned a required output width.
	// Replace all edges with a "o--[swizzle]--o" pattern.
	const wchar_t* c_swizzles[] = { L"", L"x", L"xy", L"xyz", L"xyzw" };

	RefArray< Edge > edges = shaderGraph->getEdges();
	for (RefArray< Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
	{
		const OutputPin* sourcePin = (*i)->getSource();
		const InputPin* destinationPin = (*i)->getDestination(); 

		int32_t sourceWidth = outputWidths[sourcePin].width;
		int32_t destinationWidth = inputWidths[destinationPin];

		if (sourceWidth <= destinationWidth || sourceWidth == 0 || destinationWidth == 0)
			continue;

		// \fixme Bug in shader graph; must remove edge prior to adding new edges.
		shaderGraph->removeEdge(*i);

		const std::pair< int, int > p1 = sourcePin->getNode()->getPosition();
		const std::pair< int, int > p2 = destinationPin->getNode()->getPosition();

		std::pair< int, int > p(
			(p1.first + p2.first) / 2,
			(p1.second + p2.second) / 2
		);

		Ref< Swizzle > swizzleNode = new Swizzle(c_swizzles[destinationWidth]);
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
