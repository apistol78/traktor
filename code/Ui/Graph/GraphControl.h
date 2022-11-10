/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Edge;
class IBitmap;
class INodeShape;
class Node;
class PaintSettings;
class Pin;

/*! Graph control.
 * \ingroup UI
 */
class T_DLLCLASS GraphControl : public Widget
{
	T_RTTI_CLASS;

public:
	enum Styles
	{
		WsEdgeSelectable = WsUser
	};

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

	bool create(ui::Widget* parent, int style = WsDoubleBuffer | WsAccelerated);

	virtual void destroy() override;

	Node* createNode(const std::wstring& title, const std::wstring& info, const Point& position, const INodeShape* shape);

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

	int getSelectedNodes(RefArray< Node >& out) const;

	int getSelectedEdges(RefArray< Edge >& out) const;

	/*! Get edges connected to given pin.
	 *
	 * \param pin Pin from which we want to find edges.
	 * \param outEdges Found edges.
	 * \return Number of edges.
	 */
	int getConnectedEdges(const Pin* pin, RefArray< Edge >& outEdges) const;

	/*! Get edges connected to given node.
	 *
	 * \param node Node from which we want to find edges.
	 * \param outEdges Found edges.
	 * \return Number of edges.
	 */
	int getConnectedEdges(const Node* node, RefArray< Edge >& outEdges) const;

	/*! Get edges connected to given nodes.
	 *
	 * \param nodes Nodes from which we want to find edges.
	 * \param inclusive True if both endpoints of an edge must be in nodes array, false if only one must be in the array.
	 * \param outEdges Found edges.
	 * \return Number of edges.
	 */
	int getConnectedEdges(const RefArray< Node >& nodes, bool inclusive, RefArray< Edge >& outEdges) const;

	Node* getNodeAt(const Point& p) const;

	Edge* getEdgeAt(const Point& p) const;

	Pin* getPinAt(const Point& p) const;

	void setPaintSettings(const PaintSettings* paintSettings);

	const PaintSettings* getPaintSettings() const;

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
	Ref< const PaintSettings > m_paintSettings;
	Ref< IBitmap > m_imageLabel;
	RefArray< Node > m_nodes;
	RefArray< Edge > m_edges;
	float m_scale;
	Size m_offset;
	Point m_cursor;
	Point m_moveOrigin;
	Point m_edgeOrigin;
	int m_mode;
	bool m_moveAll;
	bool m_moveSelected;
	bool m_edgeSelectable;
	Ref< Pin > m_selectedPin;
	Ref< Node > m_selectedNode;
	Ref< Edge > m_selectedEdge;
	std::vector< Point > m_nodePositions;
	std::vector< bool > m_nodeSelectionStates;
	std::vector< bool > m_edgeSelectionStates;
	const Pin* m_hotPin;

	void beginSelectModification();

	bool endSelectModification();

	void eventMouseDown(MouseButtonDownEvent* event);

	void eventMouseUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventDoubleClick(MouseDoubleClickEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

