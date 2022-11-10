/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/SmallSet.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphHash.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

uint32_t rotateLeft(uint32_t value, uint32_t count)
{
	return (value << count) | (value >> (32 - count));
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphHash", ShaderGraphHash, Object)

ShaderGraphHash::ShaderGraphHash(bool includeTextures)
:	m_includeTextures(includeTextures)
{
}

uint32_t ShaderGraphHash::calculate(const Node* node) const
{
	Ref< Node > nodeCopy = DeepClone(node).create< Node >();
	
	nodeCopy->setId(Guid::null);
	nodeCopy->setPosition(std::make_pair(0, 0));
	nodeCopy->setComment(L"");

	if (!m_includeTextures)
	{
		if (auto textureNode = dynamic_type_cast< Texture* >(nodeCopy))
			textureNode->setExternal(Guid::null);
	}

	return DeepHash(nodeCopy).get();
}

uint32_t ShaderGraphHash::calculate(const ShaderGraph* shaderGraph) const
{
	AlignedVector< std::pair< Ref< const Node >, int32_t > > nodeStack;
	SmallSet< std::pair< Ref< const Node >, int32_t > > nodeVisited;
	uint32_t hash = 0;

	// Collect root nodes; assume all nodes with no output pins to be roots.
	for (auto node : shaderGraph->getNodes())
	{
		const INodeTraits* nodeTraits = INodeTraits::find(node);
		if (nodeTraits != nullptr && nodeTraits->isRoot(shaderGraph, node))
			nodeStack.push_back(std::make_pair(node, 0));
	}

	// Traverse graph nodes.
	while (!nodeStack.empty())
	{
		std::pair< Ref< const Node >, int32_t > top = nodeStack.back();
		nodeStack.pop_back();

		// Already visited this node?
		if (nodeVisited.find(top) != nodeVisited.end())
			continue;
		nodeVisited.insert(top);

		const Node* node = top.first;
		int32_t order = top.second;

		// Find node's traits; some nodes which doesn't have a trait
		// (meta type nodes such as Branch, Type etc) we treat as each
		// input is dependent.
		const INodeTraits* nodeTraits = INodeTraits::find(node);

		// Calculate local hash.
		hash += rotateLeft(calculate(node), order);

		// Push all input nodes onto stack.
		int32_t inputPinCount = node->getInputPinCount();
		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			T_ASSERT(inputPin);

			const OutputPin* sourcePin = shaderGraph->findSourcePin(inputPin);
			if (!sourcePin)
				continue;

			const Node* childNode = sourcePin->getNode();
			int32_t childOrder = nodeTraits ? nodeTraits->getInputPinGroup(shaderGraph, node, inputPin) : i;

			nodeStack.push_back(std::make_pair(childNode, childOrder));
		}
	}

	return hash;
}

	}
}
