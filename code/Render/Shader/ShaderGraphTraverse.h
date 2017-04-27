/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ShaderGraphUtilities_H
#define traktor_render_ShaderGraphUtilities_H

#include <set>
#include "Core/RefArray.h"
#include "Render/Shader/Edge.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/OutputPin.h"
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
		return !found;
	}
};

struct PinsConnected
{
	const InputPin* inputPin;
	const OutputPin* outputPin;
	bool connected;

	PinsConnected()
	:	inputPin(0)
	,	outputPin(0)
	,	connected(false)
	{
	}

	bool operator () (Node* node)
	{
		return !connected;
	}

	bool operator () (Edge* edge)
	{
		// Don't traverse paths from source node from wrong input pin.
		if (
			edge->getDestination()->getNode() == inputPin->getNode() &&
			edge->getDestination() != inputPin
		)
			return false;

		connected |= bool(outputPin == edge->getSource());
		return true;
	}
};

struct CollectOutputs
{
	const InputPin* inputPin;
	std::vector< const OutputPin* > outputPins;

	CollectOutputs()
	:	inputPin(0)
	{
	}

	bool operator () (Node* node)
	{
		return true;
	}

	bool operator () (Edge* edge)
	{
		// Don't traverse paths from source node from wrong input pin.
		if (
			edge->getDestination()->getNode() == inputPin->getNode() &&
			edge->getDestination() != inputPin
		)
			return false;

		outputPins.push_back(edge->getSource());
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

/*! \brief Check if two pins are connected.
 * \ingroup Render
 */
inline bool arePinsConnected(const ShaderGraph* shaderGraph, const OutputPin* outputPin, const InputPin* inputPin)
{
	PinsConnected visitor;
	visitor.outputPin = outputPin;
	visitor.inputPin = inputPin;
	visitor.connected = false;
	ShaderGraphTraverse(shaderGraph, inputPin->getNode()).preorder(visitor);
	return visitor.connected;
}

/*! \brief Get merging, common, outputs from a set of input pins.
 * \ingroup Render
 */
inline void getMergingOutputs(const ShaderGraph* shaderGraph, const std::vector< const InputPin* >& inputPins, std::vector< const OutputPin* >& outMergedOutputPins)
{
	T_ASSERT (inputPins.size() >= 2);

	// Collect all reachable output pins.
	std::vector< CollectOutputs > visitors(inputPins.size());
	for (size_t i = 0; i < inputPins.size(); ++i)
	{
		visitors[i].inputPin = inputPins[i];
		ShaderGraphTraverse(shaderGraph, inputPins[i]->getNode()).preorder(visitors[i]);
	}

	// Keep only output pins which are found from all inputs.
	std::vector< const OutputPin* >& commonOutputPins = visitors[0].outputPins;
	for (size_t i = 1; i < visitors.size(); ++i)
	{
		for (size_t j = 0; j < commonOutputPins.size(); )
		{
			if (std::find(visitors[i].outputPins.begin(), visitors[i].outputPins.end(), commonOutputPins[j]) == visitors[i].outputPins.end())
				commonOutputPins.erase(commonOutputPins.begin() + j);
			else
				++j;
		}
	}

	// Keep only "right most" output pins.
	for (size_t i = 0; i < commonOutputPins.size(); ++i)
	{
		bool connected = false;

		for (size_t j = 0; j < commonOutputPins.size() && !connected; ++j)
		{
			if (i == j)
				continue;

			Node* checkNode = commonOutputPins[j]->getNode();
			int32_t checkInputPinCount = checkNode->getInputPinCount();
			for (int32_t k = 0; k < checkInputPinCount && !connected; ++k)
			{
				const InputPin* checkInputPin = checkNode->getInputPin(k);
				connected = arePinsConnected(shaderGraph, commonOutputPins[i], checkInputPin);
			}
		}

		if (!connected)
			outMergedOutputPins.push_back(commonOutputPins[i]);
	}
}

/*! \brief Get non-dependent outputs from a an input and a set of dependent output pins.
 * \ingroup Render
 */
inline void getNonDependentOutputs(const ShaderGraph* shaderGraph, const InputPin* inputPin, const std::vector< const OutputPin* >& dependentOutputPins, std::vector< const OutputPin* >& outOutputPins)
{
	CollectOutputs visitor;
	visitor.inputPin = inputPin;
	ShaderGraphTraverse(shaderGraph, inputPin->getNode()).preorder(visitor);

	// Keep only output pins which are not dependent on input from dependentOutputPins.
	std::vector< const OutputPin* > nonDependentOutputPins;
	for (std::vector< const OutputPin* >::const_iterator i = visitor.outputPins.begin(); i != visitor.outputPins.end(); ++i)
	{
		bool outputNodeDependent = false;

		Node* outputNode = (*i)->getNode();
		int32_t outputNodeInputPinCount = outputNode->getInputPinCount();
		for (int32_t j = 0; j < outputNodeInputPinCount && !outputNodeDependent; ++j)
		{
			const InputPin* outputNodeInputPin = outputNode->getInputPin(j);
			for (std::vector< const OutputPin* >::const_iterator k = dependentOutputPins.begin(); k != dependentOutputPins.end() && !outputNodeDependent; ++k)
			{
				if (arePinsConnected(shaderGraph, *k, outputNodeInputPin))
					outputNodeDependent = true;
			}
		}

		if (!outputNodeDependent)
			nonDependentOutputPins.push_back(*i);
	}

	// Keep only "right most" output pins.
	for (size_t i = 0; i < nonDependentOutputPins.size(); ++i)
	{
		bool connected = false;

		for (size_t j = 0; j < nonDependentOutputPins.size() && !connected; ++j)
		{
			if (i == j)
				continue;

			Node* checkNode = nonDependentOutputPins[j]->getNode();
			int32_t checkInputPinCount = checkNode->getInputPinCount();
			for (int32_t k = 0; k < checkInputPinCount && !connected; ++k)
			{
				const InputPin* checkInputPin = checkNode->getInputPin(k);
				connected = arePinsConnected(shaderGraph, nonDependentOutputPins[i], checkInputPin);
			}
		}

		if (!connected)
			outOutputPins.push_back(nonDependentOutputPins[i]);
	}
}

	}
}

#endif	// traktor_render_ShaderGraphUtilities_H
