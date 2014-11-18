#include <algorithm>
#include <set>
#include "Core/Log/Log.h"
#include "Render/Shader/Edge.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphCombinations.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class FindCombinationMaskValue
{
public:
	FindCombinationMaskValue(uint32_t mask, uint32_t value)
	:	m_mask(mask)
	,	m_value(value)
	{
	}

	bool operator () (const ShaderGraphCombinations::Combination& c) const
	{
		return c.mask == m_mask && c.value == m_value;
	}

private:
	uint32_t m_mask;
	uint32_t m_value;
};

Ref< ShaderGraph > replaceBranch(const ShaderGraph* shaderGraph, Branch* branch, bool path)
{
	Ref< ShaderGraph > shaderGraphResult = new ShaderGraph(
		shaderGraph->getNodes(),
		shaderGraph->getEdges()
	);

	const InputPin* inputPin = branch->getInputPin(path ? /* True */ 0 : /* False */ 1);
	T_ASSERT (inputPin);

	const OutputPin* outputPin = branch->getOutputPin(/* Output */ 0);
	T_ASSERT (outputPin);

	Ref< Edge > sourceEdge = shaderGraphResult->findEdge(inputPin);
	T_ASSERT (sourceEdge);

	RefSet< Edge > destinationEdges;
	shaderGraphResult->findEdges(outputPin, destinationEdges);

	shaderGraphResult->removeEdge(sourceEdge);
	for (RefSet< Edge >::const_iterator j = destinationEdges.begin(); j != destinationEdges.end(); ++j)
	{
		shaderGraphResult->removeEdge(*j);
		shaderGraphResult->addEdge(new Edge(
			sourceEdge->getSource(),
			(*j)->getDestination()
		));
	}

	shaderGraphResult = ShaderGraphOptimizer(shaderGraphResult).removeUnusedBranches();
	if (!shaderGraphResult)
		return 0;

	shaderGraphResult = ShaderGraphStatic(shaderGraphResult).getConstantFolded();
	if (!shaderGraphResult)
		return 0;

	return shaderGraphResult;
}

void buildCombinations(
	const ShaderGraph* shaderGraph,
	const std::vector< std::wstring >& parameterNames,
	uint32_t parameterMask,
	uint32_t parameterValue,
	std::vector< ShaderGraphCombinations::Combination >& outCombinations
)
{
	RefArray< Branch > branchNodes;
	shaderGraph->findNodesOf< Branch >(branchNodes);

	if (!branchNodes.empty())
	{
		Branch* branch = branchNodes.front();
		T_ASSERT (branch);

		std::vector< std::wstring >::const_iterator parameterIter = std::find(parameterNames.begin(), parameterNames.end(), branch->getParameterName());
		uint32_t parameterBit = 1 << uint32_t(std::distance(parameterNames.begin(), parameterIter));

		Ref< ShaderGraph > shaderGraphBranchTrue = replaceBranch(shaderGraph, branch, true);
		if (shaderGraphBranchTrue)
		{
			buildCombinations(
				shaderGraphBranchTrue,
				parameterNames,
				parameterMask | parameterBit,
				parameterValue | parameterBit,
				outCombinations
			);
		}

		Ref< ShaderGraph > shaderGraphBranchFalse = replaceBranch(shaderGraph, branch, false);
		if (shaderGraphBranchFalse)
		{
			buildCombinations(
				shaderGraphBranchFalse,
				parameterNames,
				parameterMask | parameterBit,
				parameterValue,
				outCombinations
			);
		}
	}
	else
	{
		if (std::find_if(outCombinations.begin(), outCombinations.end(), FindCombinationMaskValue(parameterMask, parameterValue)) == outCombinations.end())
		{
			Ref< ShaderGraph > shaderGraphNoBranch = ShaderGraphStatic(shaderGraph).getConstantFolded();
			if (shaderGraphNoBranch)
			{
				ShaderGraphCombinations::Combination c;
				c.mask = parameterMask;
				c.value = parameterValue;
				c.shaderGraph = shaderGraph;
				outCombinations.push_back(c);
			}
		}
	}
}

		}

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

	buildCombinations(m_shaderGraph, m_parameterNames, 0, 0, m_combinations);
}

const std::vector< std::wstring >& ShaderGraphCombinations::getParameterNames() const
{
	return m_parameterNames;
}

std::vector< std::wstring > ShaderGraphCombinations::getParameterNames(uint32_t mask) const
{
	std::vector< std::wstring > parameterNames;
	for (uint32_t i = 0; i < uint32_t(m_parameterNames.size()); ++i)
	{
		if ((mask & (1 << i)) != 0)
			parameterNames.push_back(m_parameterNames[i]);
	}
	return parameterNames;
}

uint32_t ShaderGraphCombinations::getCombinationCount() const
{
	return m_combinations.size();
}

uint32_t ShaderGraphCombinations::getCombinationMask(uint32_t index) const
{
	T_ASSERT (index < m_combinations.size());
	return m_combinations[index].mask;
}

uint32_t ShaderGraphCombinations::getCombinationValue(uint32_t index) const
{
	T_ASSERT (index < m_combinations.size());
	return m_combinations[index].value;
}

Ref< const ShaderGraph > ShaderGraphCombinations::getCombinationShaderGraph(uint32_t index) const
{
	T_ASSERT (index < m_combinations.size());
	return m_combinations[index].shaderGraph;
}

	}
}
