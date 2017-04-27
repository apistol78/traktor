/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <list>
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/INodeTraits.h"
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

uint32_t ShaderGraphHash::calculate(const Node* node)
{
	Ref< Node > nodeCopy = DeepClone(node).create< Node >();

	nodeCopy->setPosition(std::make_pair(0, 0));
	nodeCopy->setComment(L"");

	return DeepHash(nodeCopy).get();
}

uint32_t ShaderGraphHash::calculate(const ShaderGraph* shaderGraph)
{
	std::list< std::pair< Ref< const Node >, int32_t > > nodeStack;
	std::set< std::pair< Ref< const Node >, int32_t > > nodeVisited;
	uint32_t hash = 0;

	// Collect root nodes; assume all nodes with no output pins to be roots.
	const RefArray< Node >& nodes = shaderGraph->getNodes();
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if ((*i)->getOutputPinCount() <= 0)
			nodeStack.push_back(std::make_pair(*i, 0));
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

		Ref< const Node > node = top.first;
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
			T_ASSERT (inputPin);

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
