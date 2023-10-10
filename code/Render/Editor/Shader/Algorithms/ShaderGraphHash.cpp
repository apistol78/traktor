/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/SmallSet.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Script.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphHash.h"

namespace traktor::render
{
	namespace
	{

uint32_t rotateLeft(uint32_t value, uint32_t count)
{
	T_FATAL_ASSERT(count < 32);
	return (value << count) | (value >> (32 - count));
}

uint32_t calculateLocalNodeHash(const Node* node, bool includeTextures, bool includeTechniqueNames)
{
	Ref< Node > nodeCopy = DeepClone(node).create< Node >();
	
	nodeCopy->setId(Guid::null);
	nodeCopy->setPosition(std::make_pair(0, 0));
	nodeCopy->setComment(L"");

	if (!includeTextures)
	{
		if (auto textureNode = dynamic_type_cast< Texture* >(nodeCopy))
			textureNode->setExternal(Guid::null);
	}

	if (!includeTechniqueNames)
	{
		if (auto vertexNode = dynamic_type_cast< VertexOutput* >(nodeCopy))
			vertexNode->setTechnique(L"");
		else if (auto pixelNode = dynamic_type_cast< PixelOutput* >(nodeCopy))
			pixelNode->setTechnique(L"");
		else if (auto computeNode = dynamic_type_cast< ComputeOutput* >(nodeCopy))
			computeNode->setTechnique(L"");
		else if (auto scriptNode = dynamic_type_cast< Script* >(nodeCopy))
			scriptNode->setTechnique(L"");
	}

	return DeepHash(nodeCopy).get();
}

uint32_t calculateBranchHash(const ShaderGraph* shaderGraph, const OutputPin* outputPin, SmallMap< const OutputPin*, uint32_t >& visited)
{
	const Node* node = outputPin->getNode();
	const INodeTraits* nodeTraits = INodeTraits::find(node);

	uint32_t nodeHash = calculateLocalNodeHash(node, false, false);

	const uint32_t inputPinCount = node->getInputPinCount();
	for (uint32_t i = 0; i < inputPinCount; ++i)
	{
		const InputPin* inputPin = node->getInputPin(i);
		T_ASSERT(inputPin);

		const int32_t inputPinGroup = nodeTraits ? nodeTraits->getInputPinGroup(shaderGraph, node, inputPin) : i;

		const Edge* edge = shaderGraph->findEdge(inputPin);
		if (!edge)
			continue;

		const auto it = visited.find(edge->getSource());
		if (it != visited.end())
		{
			// Hash already calculated for this output.
			nodeHash += rotateLeft(it->second, inputPinGroup);
		}
		else
		{
			visited[edge->getSource()] = 0;
			const uint32_t inputNodeHash = calculateBranchHash(shaderGraph, edge->getSource(), visited);
			visited[edge->getSource()] = inputNodeHash;
			nodeHash += rotateLeft(inputNodeHash, inputPinGroup);
		}
	}

	for (int32_t i = 0; i < node->getOutputPinCount(); ++i)
	{
		if (node->getOutputPin(i) == outputPin)
		{
			nodeHash = rotateLeft(nodeHash, i);
			break;
		}
	}

	return nodeHash;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphHash", ShaderGraphHash, Object)

ShaderGraphHash::ShaderGraphHash(bool includeTextures, bool includeTechniqueNames)
:	m_includeTextures(includeTextures)
,	m_includeTechniqueNames(includeTechniqueNames)
{
}

uint32_t ShaderGraphHash::calculate(const Node* node) const
{
	return calculateLocalNodeHash(node, m_includeTextures, m_includeTechniqueNames);
}

uint32_t ShaderGraphHash::calculate(const ShaderGraph* shaderGraph) const
{
	SmallMap< const OutputPin*, uint32_t > visited;
	uint32_t hash = 0;

	for (auto node : shaderGraph->getNodes())
	{
		const INodeTraits* nodeTraits = INodeTraits::find(node);
		if (nodeTraits != nullptr && nodeTraits->isRoot(shaderGraph, node))
		{
			hash += calculateLocalNodeHash(node, false, false);

			for (int32_t i = 0; i < node->getInputPinCount(); ++i)
			{
				const InputPin* inputPin = node->getInputPin(i);
				const Edge* inputEdge = shaderGraph->findEdge(inputPin);
				if (inputEdge)
				{
					const uint32_t inputHash = calculateBranchHash(shaderGraph, inputEdge->getSource(), visited);
					const int32_t inputPinGroup = nodeTraits->getInputPinGroup(shaderGraph, node, inputPin);
					hash += rotateLeft(inputHash, inputPinGroup);
				}
			}
		}
	}

	return hash;
}

}
