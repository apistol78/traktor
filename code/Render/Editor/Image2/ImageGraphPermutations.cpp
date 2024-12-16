/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <set>
#include "Core/Log/Log.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Image2/ImageGraphAsset.h"
#include "Render/Editor/Image2/ImageGraphPermutations.h"
#include "Render/Editor/Image2/ImgPermutation.h"

namespace traktor::render
{
	namespace
	{

Ref< ImageGraphAsset > replaceBranch(const ImageGraphAsset* shaderGraph, ImgPermutation* branch, bool path)
{
	Ref< ImageGraphAsset > imageGraphResult = new ImageGraphAsset(
		shaderGraph->getNodes(),
		shaderGraph->getEdges()
	);

	const InputPin* inputPin = branch->getInputPin(path ? /* True */ 0 : /* False */ 1);
	T_ASSERT(inputPin);

	const OutputPin* outputPin = branch->getOutputPin(/* Output */ 0);
	T_ASSERT(outputPin);

	Ref< Edge > sourceEdge = imageGraphResult->findEdge(inputPin);
	if (!sourceEdge)
		return nullptr;

	RefArray< Edge > destinationEdges = imageGraphResult->findEdges(outputPin);

	imageGraphResult->removeEdge(sourceEdge);
	for (auto destinationEdge : destinationEdges)
	{
		imageGraphResult->removeEdge(destinationEdge);
		imageGraphResult->addEdge(new Edge(
			sourceEdge->getSource(),
			destinationEdge->getDestination()
		));
	}

	imageGraphResult->removeNode(branch);

	return imageGraphResult;
}

void buildPermutations(
	const ImageGraphAsset* imageGraph,
	const AlignedVector< std::wstring >& parameterNames,
	uint32_t parameterMask,
	uint32_t parameterValue,
	AlignedVector< ImageGraphPermutations::Permutation >& outCombinations
)
{
	RefArray< ImgPermutation > branchNodes = imageGraph->findNodesOf< ImgPermutation >();
	if (!branchNodes.empty())
	{
		ImgPermutation* branch = branchNodes.front();
		T_ASSERT(branch);

		const auto parameterIter = std::find(parameterNames.begin(), parameterNames.end(), branch->getName());
		const uint32_t parameterBit = 1 << (uint32_t)std::distance(parameterNames.begin(), parameterIter);

		Ref< ImageGraphAsset > imageGraphBranchTrue = replaceBranch(imageGraph, branch, true);
		if (imageGraphBranchTrue)
		{
			buildPermutations(
				imageGraphBranchTrue,
				parameterNames,
				parameterMask | parameterBit,
				parameterValue | parameterBit,
				outCombinations
			);
		}

		Ref< ImageGraphAsset > imageGraphBranchFalse = replaceBranch(imageGraph, branch, false);
		if (imageGraphBranchFalse)
		{
			buildPermutations(
				imageGraphBranchFalse,
				parameterNames,
				parameterMask | parameterBit,
				parameterValue,
				outCombinations
			);
		}
	}
	else
	{
		const auto it = std::find_if(outCombinations.begin(), outCombinations.end(), [=](const ImageGraphPermutations::Permutation& c) {
			return c.mask == parameterMask && c.value == parameterValue;
		});
		if (it == outCombinations.end())
		{
			auto& p = outCombinations.push_back();
			p.mask = parameterMask;
			p.value = parameterValue;
			p.imageGraph = imageGraph;
		}
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageGraphPermutations", ImageGraphPermutations, Object)

ImageGraphPermutations::ImageGraphPermutations(const ImageGraphAsset* imageGraph)
{
	m_imageGraph = imageGraph;

	RefArray< ImgPermutation > branchNodes = m_imageGraph->findNodesOf< ImgPermutation >();

	std::set< std::wstring > parameterNames;
	for (auto branchNode : branchNodes)
	{
		const std::wstring name = branchNode->getName();
		if (parameterNames.find(name) != parameterNames.end())
			continue;

		m_parameterNames.push_back(name);
		parameterNames.insert(name);
	}

	buildPermutations(m_imageGraph, m_parameterNames, 0, 0, m_permutations);
}

const AlignedVector< std::wstring >& ImageGraphPermutations::getParameterNames() const
{
	return m_parameterNames;
}

AlignedVector< std::wstring > ImageGraphPermutations::getParameterNames(uint32_t mask) const
{
	AlignedVector< std::wstring > parameterNames;
	for (uint32_t i = 0; i < uint32_t(m_parameterNames.size()); ++i)
	{
		if ((mask & (1 << i)) != 0)
			parameterNames.push_back(m_parameterNames[i]);
	}
	return parameterNames;
}

uint32_t ImageGraphPermutations::getPermutationCount() const
{
	return (uint32_t)m_permutations.size();
}

uint32_t ImageGraphPermutations::getPermutationMask(uint32_t index) const
{
	T_ASSERT(index < m_combinations.size());
	return m_permutations[index].mask;
}

uint32_t ImageGraphPermutations::getPermutationValue(uint32_t index) const
{
	T_ASSERT(index < m_combinations.size());
	return m_permutations[index].value;
}

Ref< const ImageGraphAsset > ImageGraphPermutations::getPermutationImageGraph(uint32_t index) const
{
	T_ASSERT(index < m_combinations.size());
	return m_permutations[index].imageGraph;
}

}
