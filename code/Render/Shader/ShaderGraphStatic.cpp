#include "Render/Shader/Edge.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/ShaderGraphOptimizer.h"
#include "Render/Shader/ShaderGraphStatic.h"

namespace traktor
{
	namespace render
	{

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

	}
}
