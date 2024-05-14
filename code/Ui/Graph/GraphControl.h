/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/RefArray.h"
#include "Ui/Point.h"
#include "Ui/Widget.h"
#include "Ui/Graph/PaintSettings.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Edge;
class Group;
class IBitmap;
class INodeShape;
class Node;
class Pin;

/*! Graph control.
 * \ingroup UI
 */
class T_DLLCLASS GraphControl : public Widget
{
	T_RTTI_CLASS;

public:
	constexpr static uint32_t WsEdgeSelectable = WsUser;

	enum Alignment
	{
		AnLeft,
		AnTop,
		AnRight,
		AnBottom
	};

	enum EvenSpace
	{
		EsHorizontally,
		EsVertically
	};

	GraphControl();

	bool create(Widget* parent, uint32_t style = WsDoubleBuffer | WsAccelerated);

	virtual void destroy() override;

	Group* createGroup(const std::wstring& title, const UnitPoint& position, const UnitSize& size);

	void removeGroup(Group* group);

	void removeAllGroups();

	RefArray< Group >& getGroups();

	const RefArray< Group >& getGroups() const;

	Node* createNode(const std::wstring& title, const std::wstring& info, const UnitPoint& position, const INodeShape* shape);

	void removeNode(Node* node);

	void removeAllNodes();

	RefArray< Node >& getNodes();

	const RefArray< Node >& getNodes() const;

	void selectAllNodes();

	void deselectAllNodes();

	void addEdge(Edge* edge);

	void removeEdge(Edge* edge);

	void removeAllEdges();

	RefArray< Edge >& getEdges();

	const RefArray< Edge >& getEdges() const;

	RefArray< Group > getSelectedGroups() const;

	RefArray< Node > getSelectedNodes() const;

	RefArray< Edge > getSelectedEdges() const;

	/*! Get edges connected to given pin.
	 *
	 * \param pin Pin from which we want to find edges.
	 * \return Found edges.
	 */
	RefArray< Edge > getConnectedEdges(const Pin* pin) const;

	/*! Get edges connected to given node.
	 *
	 * \param node Node from which we want to find edges.
	 * \return Found edges.
	 */
	RefArray< Edge > getConnectedEdges(const Node* node) const;

	/*! Get edges connected to given nodes.
	 *
	 * \param nodes Nodes from which we want to find edges.
	 * \param inclusive True if both endpoints of an edge must be in nodes array, false if only one must be in the array.
	 * \return Found edges.
	 */
	RefArray< Edge > getConnectedEdges(const RefArray< Node >& nodes, bool inclusive) const;

	Group* getGroupAt(const Point& p) const;

	Node* getNodeAt(const Point& p) const;

	Edge* getEdgeAt(const Point& p) const;

	Pin* getPinAt(const Point& p) const;

	/*! Add a dependency hint between two nodes.
	 * 
	 * \param fromNode Dependency from node.
	 * \param toNode Dependency to node.
	 */
	void addDependencyHint(const Node* fromNode, const Node* toNode);

	void removeDependencyHint(const Node* fromNode, const Node* toNode);

	void removeAllDependencyHints();

	void setPaintSettings(const PaintSettings& paintSettings);

	const PaintSettings& getPaintSettings() const;

	void setScale(float scale);

	float getScale() const;

	void center(bool selectedOnly = false);

	void alignNodes(Alignment align);

	void evenSpace(EvenSpace space);

	Size getOffset() const;

	Point clientToVirtual(const Point& cpt) const;

	Point virtualToClient(const Point& vpt) const;

	Rect getVirtualRect() const;

private:
	PaintSettings m_paintSettings;
	Ref< IBitmap > m_imageLabel;
	RefArray< Group > m_groups;		//!< All groups in graph.
	RefArray< Node > m_nodes;		//!< All nodes in graph.
	RefArray< Edge > m_edges;		//!< All edges in graph.
	float m_scale;
	Size m_offset;
	Point m_cursor;
	Point m_moveOrigin;
	Point m_edgeOrigin;
	int32_t m_mode;
	int32_t m_groupAnchor;
	bool m_edgeSelectable;
	Ref< Pin > m_selectedPin;
	std::vector< UnitRect > m_groupPositions;
	std::vector< UnitRect > m_nodePositions;
	std::vector< bool > m_groupSelectionStates;
	std::vector< bool > m_nodeSelectionStates;
	std::vector< bool > m_edgeSelectionStates;
	std::vector< std::pair< const Node*, const Node* > > m_dependencyHints;
	const Pin* m_hotPin;
	const Edge* m_hotEdge;

	void beginSelectModification();

	bool endSelectModification();

	void capturePositions();

	void eventMouseDown(MouseButtonDownEvent* event);

	void eventMouseUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventDoubleClick(MouseDoubleClickEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);

	void eventPaint(PaintEvent* event);

	void eventSize(SizeEvent* event);
};

}
