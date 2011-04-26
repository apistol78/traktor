#ifndef traktor_render_ShaderGraphUtilities_H
#define traktor_render_ShaderGraphUtilities_H

#include <set>
#include "Core/RefArray.h"
#include "Render/Shader/Edge.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{

/*! \brief Traverse shader graph nodes through a root set.
 * \ingroup Render
 *
 * \param shaderGraph Shader graph.
 * \param roots Root nodes.
 * \param visitor Node and edge visitor.
 */
template < typename VisitorType >
void shaderGraphTraverse(const ShaderGraph* shaderGraph, const RefArray< Node >& roots, VisitorType& visitor)
{
	std::set< const Node* > nodeVisited;
	RefArray< Node > nodeStack;

	// Push root nodes.
	nodeStack = roots;

	// Traverse graph nodes.
	while (!nodeStack.empty())
	{
		Node* node = nodeStack.back();
		nodeStack.pop_back();

		// Already visited this node?
		if (nodeVisited.find(node) != nodeVisited.end())
			continue;
		nodeVisited.insert(node);
		
		// Invoke visitors with this node.
		if (!visitor(node))
			continue;

		// Push all input nodes onto stack.
		int32_t inputPinCount = node->getInputPinCount();
		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			T_ASSERT (inputPin);

			Edge* edge = shaderGraph->findEdge(inputPin);
			if (edge)
			{
				// Invoke visitor with this edge.
				if (!visitor(edge))
					continue;
				
				// Push node onto stack.
				nodeStack.push_back(edge->getSource()->getNode());
			}
		}
	}
}

	}
}

#endif	// traktor_render_ShaderGraphUtilities_H
