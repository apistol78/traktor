#include <algorithm>
#include <set>
#include "Render/Shader/Edge.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphCombinations.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphCombinations", ShaderGraphCombinations, Object)

ShaderGraphCombinations::ShaderGraphCombinations(const ShaderGraph* shaderGraph)
{
	m_shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches();

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

		Ref< Edge > sourceEdge = shaderGraph->findEdge(inputPin);
		T_ASSERT (sourceEdge);

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
