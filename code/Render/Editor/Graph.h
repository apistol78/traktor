/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/RefSet.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Edge;
class Group;
class InputPin;
class Node;
class OutputPin;

/*! Generic node/edge-style graph.
 * \ingroup Render
 */
class T_DLLCLASS Graph : public ISerializable
{
	T_RTTI_CLASS;

public:
	Graph() = default;

	explicit Graph(const RefArray< Node >& nodes, const RefArray< Edge >& edges);

	virtual ~Graph();

	/*! Add node to graph.
	 *
	 * \param node Node to add.
	 */
	void addNode(Node* node);

	/*! Remove node from graph.
	 *
	 * \param node Node to remove.
	 */
	void removeNode(Node* node);

	/*! Add edge to graph.
	 *
	 * \param edge Edge to add.
	 */
	void addEdge(Edge* edge);

	/*! Remove edge from graph.
	 *
	 * \param edge Edge to remove.
	 */
	void removeEdge(Edge* edge);

	/*!
	 */
	void addGroup(Group* group);

	/*!
	 */
	void removeGroup(Group* group);

	/*! Remove all nodes, edges and groups from graph. */
	void removeAll();

	/*! Get all nodes of a specific type.
	 *
	 * \param nodeType Type of node to find.
	 * \param outNodes Nodes of given type.
	 * \return Number of nodes.
	 */
	size_t findNodesOf(const TypeInfo& nodeType, RefArray< Node >& outNodes) const;

	/*! Get all nodes of a specific type.
	 *
	 * \param nodeType Type of node to find.
	 * \param outNodes Nodes of given type.
	 * \return Number of nodes.
	 */
	template < typename NodeType >
	RefArray< NodeType > findNodesOf() const
	{
		RefArray< Node > untypedNodes;
		findNodesOf(type_of< NodeType >(), untypedNodes);
		return (RefArray< NodeType >&&)untypedNodes;
	}

	/*! Find edge connected to input pin.
	 *
	 * \param inputPin Input pin.
	 * \return Connected edge.
	 */
	Edge* findEdge(const InputPin* inputPin) const;

	/*! Find edges connected from output pin.
	 *
	 * \param outputPin Output pin.
	 * \param outEdges Connected edges.
	 * \return Number of connected edges.
	 */
	RefArray< Edge > findEdges(const OutputPin* outputPin) const;

	/*! Find output pin connected to input pin.
	 *
	 * \param inputPin Input pin.
	 * \return Connected output pin.
	 */
	const OutputPin* findSourcePin(const InputPin* inputPin) const;

	/*! Find all input pins which are connected to output pin.
	 *
	 * \param outputPin Output pin.
	 * \param outDestinations Connected input pins.
	 * \return Number of connected input pins.
	 */
	uint32_t findDestinationPins(const OutputPin* outputPin, AlignedVector< const InputPin* >& outDestinations) const;

	/*! Get number of destination pins connected to an output pin.
	 *
	 * \param outputPin Output pin.
	 * \return Number of connected destinations.
	 */
	uint32_t getDestinationCount(const OutputPin* outputPin) const;

	/*! Detach node from all input and output edges.
	 *
	 * \param node Node to be detached.
	 */
	void detach(const Node* node);

	/*! Rewire all edges from one output to another output.
	 *
	 * \param outputPin Current output pin.
	 * \param newOutputPin New output pin.
	 */
	void rewire(const OutputPin* outputPin, const OutputPin* newOutputPin);

	/*! Replace old node with new node.
	 *
	 * Edges with matching pin names are
	 * kept and rewired to new node.
	 *
	 * \param oldNode Old node to be replaced.
	 * \param newNode New node replacing old node.
	 */
	void replace(Node* oldNode, Node* newNode);

	/*! Serialize graph. */
	virtual void serialize(ISerializer& s) override final;

	/*! Get all nodes.
	 *
	 * \return Array of nodes.
	 */
	const RefArray< Node >& getNodes() const { return m_nodes; }

	/*! Get all edges.
	 *
	 * \return Array of edges.
	 */
	const RefArray< Edge >& getEdges() const { return m_edges; }

	/*! Get all groups
	 * 
	 * \return Array of groups.
	 */
	const RefArray< Group >& getGroups() const { return m_groups; }

private:
	RefArray< Node > m_nodes;
	RefArray< Edge > m_edges;
	RefArray< Group > m_groups;
	SmallMap< const InputPin*, Edge* > m_inputPinToEdge;
	SmallMap< const OutputPin*, uint32_t > m_outputPinDestinationCount;

	void updateInputPinToEdge();

	void updateOutputPinDestinationCount();
};

}
