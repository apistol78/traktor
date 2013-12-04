#ifndef traktor_render_ShaderGraphUtilities_H
#define traktor_render_ShaderGraphUtilities_H

#include <set>
#include "Core/RefArray.h"
#include "Render/Shader/Edge.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{

/*! \brief Shader graph traversal helper.
 * \ingroup Render
 */
class ShaderGraphTraverse
{
public:
	ShaderGraphTraverse(const ShaderGraph* shaderGraph, Node* root)
	:	m_shaderGraph(shaderGraph)
	{
		m_roots.push_back(root);
	}

	ShaderGraphTraverse(const ShaderGraph* shaderGraph, const RefArray< Node >& roots)
	:	m_shaderGraph(shaderGraph)
	,	m_roots(roots)
	{
	}

	template < typename VisitorType >
	void preorder(VisitorType& visitor)
	{
		std::set< Node* > visited;
		for (RefArray< Node >::const_iterator i = m_roots.begin(); i != m_roots.end(); ++i)
			preorderImpl(*i, visitor, visited);
	}

	template < typename VisitorType >
	void postorder(VisitorType& visitor)
	{
		std::set< Node* > visited;
		for (RefArray< Node >::const_iterator i = m_roots.begin(); i != m_roots.end(); ++i)
			postorderImpl(*i, visitor, visited);
	}

	template < typename VisitorType >
	void breadthFirst(VisitorType& visitor)
	{
		std::set< Node* > visited;
		breadthFirstImpl(m_roots, visitor, visited);
	}

private:
	const ShaderGraph* m_shaderGraph;
	RefArray< Node > m_roots;

	template < typename VisitorType >
	void preorderImpl(Node* node, VisitorType& visitor, std::set< Node* >& visited)
	{
		if (visited.find(node) != visited.end())
			return;
		visited.insert(node);

		if (!visitor(node))
			return;

		uint32_t inputPinCount = node->getInputPinCount();
		for (uint32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			T_ASSERT (inputPin);

			Edge* edge = m_shaderGraph->findEdge(inputPin);
			if (edge)
			{
				if (!visitor(edge))
					continue;

				preorderImpl(edge->getSource()->getNode(), visitor, visited);
			}
		}
	}

	template < typename VisitorType >
	void postorderImpl(Node* node, VisitorType& visitor, std::set< Node* >& visited)
	{
		if (visited.find(node) != visited.end())
			return;
		visited.insert(node);

		uint32_t inputPinCount = node->getInputPinCount();
		for (uint32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			T_ASSERT (inputPin);

			Edge* edge = m_shaderGraph->findEdge(inputPin);
			if (edge)
			{
				if (!visitor(edge))
					continue;

				postorderImpl(edge->getSource()->getNode(), visitor, visited);
			}
		}

		if (!visitor(node))
			return;
	}

	template < typename VisitorType >
	void breadthFirstImpl(const RefArray< Node >& nodes, VisitorType& visitor, std::set< Node* >& visited)
	{
		RefArray< Node > children;
		for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			if (visited.find(*i) != visited.end())
				continue;
			visited.insert(*i);

			if (!visitor(*i))
				continue;

			uint32_t inputPinCount = (*i)->getInputPinCount();
			for (uint32_t j = 0; j < inputPinCount; ++j)
			{
				const InputPin* inputPin = (*i)->getInputPin(j);
				T_ASSERT (inputPin);

				Edge* edge = m_shaderGraph->findEdge(inputPin);
				if (edge)
				{
					if (!visitor(edge))
						continue;

					children.push_back(edge->getSource()->getNode());
				}
			}
		}
		if (!children.empty())
			breadthFirstImpl(children, visitor, visited);
	}
};

struct FindInputPin
{
	const InputPin* inputPin;
	bool found;

	FindInputPin()
	:	inputPin(0)
	,	found(false)
	{
	}

	bool operator () (Node* node)
	{
		found |= bool(inputPin->getNode() == node);
		return !found;
	}

	bool operator () (Edge* edge)
	{
		return true;
	}
};

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
	ShaderGraphTraverse(shaderGraph, roots).preorder(visitor);
}

/*! \brief Check if an input propagate to a target node.
 * \ingroup Render
 */
inline bool doesInputPropagateToNode(const ShaderGraph* shaderGraph, const InputPin* inputPin, Node* targetNode)
{
	FindInputPin visitor;
	visitor.inputPin = inputPin;
	visitor.found = false;
	ShaderGraphTraverse(shaderGraph, targetNode).preorder(visitor);
	return visitor.found;
}

	}
}

#endif	// traktor_render_ShaderGraphUtilities_H
