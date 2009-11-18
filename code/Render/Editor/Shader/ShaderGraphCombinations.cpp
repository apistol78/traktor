#include <algorithm>
#include <set>
#include "Render/Editor/Shader/ShaderGraphCombinations.h"
#include "Render/ShaderGraph.h"
#include "Render/ShaderGraphAdjacency.h"
#include "Render/Nodes.h"
#include "Render/Edge.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphCombinations", ShaderGraphCombinations, Object)

ShaderGraphCombinations::ShaderGraphCombinations(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
	RefArray< Branch > branchNodes;
	m_shaderGraph->findNodesOf< Branch >(branchNodes);

	std::set< std::wstring > parameterNames;
	for (RefArray< Branch >::iterator i = branchNodes.begin(); i != branchNodes.end(); ++i)
	{
		std::wstring name = (*i)->getParameterName();
		if (parameterNames.find(name) != parameterNames.end())
			continue;

		m_parameterNames.push_back(name);
		parameterNames.insert(name);
	}
}

const std::vector< std::wstring >& ShaderGraphCombinations::getParameterNames() const
{
	return m_parameterNames;
}

uint32_t ShaderGraphCombinations::getCombinationCount() const
{
	return 1 << uint32_t(m_parameterNames.size());
}

std::vector< std::wstring > ShaderGraphCombinations::getParameterCombination(uint32_t combination) const
{
	std::vector< std::wstring > parameterCombination;
	for (uint32_t i = 0; i < uint32_t(m_parameterNames.size()); ++i)
	{
		if ((combination & (1 << i)) != 0)
			parameterCombination.push_back(m_parameterNames[i]);
	}
	return parameterCombination;
}

Ref< ShaderGraph > ShaderGraphCombinations::generate(uint32_t combination) const
{
	Ref< ShaderGraph > shaderGraph = new ShaderGraph(
		m_shaderGraph->getNodes(),
		m_shaderGraph->getEdges()
	);
	Ref< ShaderGraphAdjacency > shaderGraphAdj = new ShaderGraphAdjacency(shaderGraph);

	RefArray< Branch > branches;
	shaderGraph->findNodesOf< Branch >(branches);

	for (RefArray< Branch >::const_iterator i = branches.begin(); i != branches.end(); ++i)
	{
		std::vector< std::wstring >::const_iterator parameterIter = std::find(m_parameterNames.begin(), m_parameterNames.end(), (*i)->getParameterName());
		uint32_t combinationMask = 1 << uint32_t(std::distance(m_parameterNames.begin(), parameterIter));

		const InputPin* inputPin = (*i)->getInputPin((combination & combinationMask) == combinationMask ? /* True */ 0 : /* False */ 1);
		T_ASSERT (inputPin);

		const OutputPin* outputPin = (*i)->getOutputPin(/* Output */ 0);
		T_ASSERT (outputPin);

		Ref< Edge > inputEdge = shaderGraphAdj->findEdge(inputPin);
		T_ASSERT (inputEdge);

		shaderGraph->removeEdge(inputEdge);

		RefArray< Edge > outputEdges;
		shaderGraphAdj->findEdges(outputPin, outputEdges);

		for (RefArray< Edge >::iterator j = outputEdges.begin(); j != outputEdges.end(); ++j)
		{
			shaderGraph->addEdge(new Edge(
				inputEdge->getSource(),
				(*j)->getDestination()
			));
			shaderGraph->removeEdge(*j);
		}

		shaderGraphAdj = new ShaderGraphAdjacency(shaderGraph);
	}

	return shaderGraph;
}

	}
}
