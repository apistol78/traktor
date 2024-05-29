/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Misc/EnterLeave.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/EdgeConnectEvent.h"
#include "Ui/Graph/EdgeDisconnectEvent.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/GraphCanvas.h"
#include "Ui/Graph/Group.h"
#include "Ui/Graph/GroupMovedEvent.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeActivateEvent.h"
#include "Ui/Graph/NodeMovedEvent.h"
#include "Ui/Graph/Pin.h"
#include "Ui/Graph/SelectEvent.h"

namespace traktor::ui
{
	namespace
	{

enum Modes
{
	MdNothing,
	MdMoveSelected,
	MdMoveGraph,
	MdDrawEdge,
	MdConnectEdge,
	MdDrawSelectionRectangle
};

struct SortNodePred
{
	GraphControl::EvenSpace m_space;

	SortNodePred(GraphControl::EvenSpace space)
	:	m_space(space)
	{
	}

	bool operator () (const Node* n1, const Node* n2) const
	{
		const UnitPoint pt1 = n1->calculateRect().getTopLeft();
		const UnitPoint pt2 = n2->calculateRect().getTopLeft();
		return m_space == GraphControl::EsHorizontally ? pt1.x < pt2.x : pt1.y < pt2.y;
	}
};

Point operator * (const Point& pt, float scale)
{
	return Point(
		(int32_t)std::floor(pt.x * scale),
		(int32_t)std::floor(pt.y * scale)
	);
}

Point operator / (const Point& pt, float scale)
{
	return pt * (1.0f / scale);
}

Rect operator * (const Rect& rc, float scale)
{
	return Rect(
		rc.getTopLeft() * scale,
		rc.getBottomRight() * scale
	);
}

Rect operator / (const Rect& rc, float scale)
{
	return rc * (1.0f / scale);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GraphControl", GraphControl, Widget)

GraphControl::GraphControl()
:	m_scale(1.0f)
,	m_mode(MdNothing)
,	m_edgeSelectable(false)
,	m_hotPin(nullptr)
,	m_hotEdge(nullptr)
{
}

bool GraphControl::create(Widget* parent, uint32_t style)
{
	if (!Widget::create(parent, style | WsFocus))
		return false;

	m_paintSettings = PaintSettings(getFont());
	m_imageLabel = new ui::StyleBitmap(L"UI.Graph.Label");

	addEventHandler< MouseButtonDownEvent >(this, &GraphControl::eventMouseDown);
	addEventHandler< MouseButtonUpEvent >(this, &GraphControl::eventMouseUp);
	addEventHandler< MouseMoveEvent >(this, &GraphControl::eventMouseMove);
	addEventHandler< MouseDoubleClickEvent >(this, &GraphControl::eventDoubleClick);
	addEventHandler< MouseWheelEvent >(this, &GraphControl::eventMouseWheel);
	addEventHandler< PaintEvent >(this, &GraphControl::eventPaint);
	addEventHandler< SizeEvent >(this, &GraphControl::eventSize);

	m_scale = 1.0f;
	m_offset.cx =
	m_offset.cy = 0;
	m_moveOrigin.x =
	m_moveOrigin.y = 0;
	m_edgeOrigin.x =
	m_edgeOrigin.y = 0;
	m_cursor.x =
	m_cursor.y = 0;
	m_mode = MdNothing;
	m_edgeSelectable = bool((style & WsEdgeSelectable) == WsEdgeSelectable);

	return true;
}

void GraphControl::destroy()
{
	for (auto node : m_nodes)
		node->m_owner = nullptr;

	m_nodes.clear();
	m_edges.clear();

	Widget::destroy();
}

Group* GraphControl::createGroup(const std::wstring& title, const UnitPoint& position, const UnitSize& size)
{
	Ref< Group > group = new Group(this, title, position, size);
	m_groups.push_back(group);
	return group;
}

void GraphControl::removeGroup(Group* group)
{
	m_groups.remove(group);
}

void GraphControl::removeAllGroups()
{
	m_groups.resize(0);
}

RefArray< Group >& GraphControl::getGroups()
{
	return m_groups;
}

const RefArray< Group >& GraphControl::getGroups() const
{
	return m_groups;
}

Node* GraphControl::createNode(const std::wstring& title, const std::wstring& info, const UnitPoint& position, const INodeShape* shape)
{
	Ref< Node > node = new Node(this, title, info, position, shape);
	m_nodes.insert(m_nodes.begin(), node);
	return node;
}

void GraphControl::removeNode(Node* node)
{
	T_FATAL_ASSERT(node->m_owner == this);
	
	node->m_owner = nullptr;
	m_nodes.remove(node);

	auto it = std::remove_if(m_dependencyHints.begin(), m_dependencyHints.end(), [=](const std::pair< const Node*, const Node* >& v) {
		return v.first == node || v.second == node;
	});
	m_dependencyHints.erase(it, m_dependencyHints.end());
}

void GraphControl::removeAllNodes()
{
	for (auto node : m_nodes)
		node->m_owner = nullptr;

	m_nodes.resize(0);
	m_edges.resize(0);
	m_dependencyHints.resize(0);
}

RefArray< Node >& GraphControl::getNodes()
{
	return m_nodes;
}

const RefArray< Node >& GraphControl::getNodes() const
{
	return m_nodes;
}

void GraphControl::selectAllNodes()
{
	for (auto node : m_nodes)
		node->setSelected(true);
}

void GraphControl::deselectAllNodes()
{
	for (auto node : m_nodes)
		node->setSelected(false);
}

void GraphControl::addEdge(Edge* edge)
{
	m_edges.push_back(edge);
}

void GraphControl::removeEdge(Edge* edge)
{
	m_edges.remove(edge);
}

void GraphControl::removeAllEdges()
{
	m_edges.resize(0);
}

RefArray< Edge >& GraphControl::getEdges()
{
	return m_edges;
}

const RefArray< Edge >& GraphControl::getEdges() const
{
	return m_edges;
}

RefArray< Group > GraphControl::getSelectedGroups() const
{
	RefArray< Group > out;
	for (auto group : m_groups)
	{
		if (group->isSelected())
			out.push_back(group);
	}
	return out;
}

RefArray< Node > GraphControl::getSelectedNodes() const
{
	RefArray< Node > out;
	for (auto node : m_nodes)
	{
		if (node->isSelected())
			out.push_back(node);
	}
	return out;
}

RefArray< Edge > GraphControl::getSelectedEdges() const
{
	RefArray< Edge > out;
	for (auto edge : m_edges)
	{
		if (edge->isSelected())
			out.push_back(edge);
	}
	return out;
}

RefArray< Edge > GraphControl::getConnectedEdges(const Pin* pin) const
{
	RefArray< Edge > edges;
	for (auto edge : m_edges)
	{
		if (edge->getSourcePin() == pin || edge->getDestinationPin() == pin)
			edges.push_back(edge);
	}
	return edges;
}

RefArray< Edge > GraphControl::getConnectedEdges(const Node* node) const
{
	RefArray< Edge > edges;
	for (auto edge : m_edges)
	{
		if (edge->getSourcePin()->getNode() == node || edge->getDestinationPin()->getNode() == node)
			edges.push_back(edge);
	}
	return edges;
}

RefArray< Edge > GraphControl::getConnectedEdges(const RefArray< Node >& nodes, bool inclusive) const
{
	RefArray< Edge > edges;
	for (auto edge : m_edges)
	{
		const bool n1 = bool(std::find(nodes.begin(), nodes.end(), edge->getSourcePin()->getNode()) != nodes.end());
		const bool n2 = bool(std::find(nodes.begin(), nodes.end(), edge->getDestinationPin()->getNode()) != nodes.end());
		if ((inclusive && (n1 && n2)) || (!inclusive && (n1 || n2)))
			edges.push_back(edge);
	}
	return edges;
}

Group* GraphControl::getGroupAt(const Point& p) const
{
	const UnitPoint up = unit(p);
	for (auto group : m_groups)
	{
		if (group->hit(up))
			return group;
	}
	return nullptr;
}

Node* GraphControl::getNodeAt(const Point& p) const
{
	const UnitPoint up = unit(p);
	for (auto node : m_nodes)
	{
		if (node->hit(up))
			return node;
	}
	return nullptr;
}

Edge* GraphControl::getEdgeAt(const Point& p) const
{
	const UnitPoint up = unit(p - m_offset);
	for (auto edge : m_edges)
	{
		if (edge->hit(this, up))
			return edge;
	}
	return nullptr;
}

Pin* GraphControl::getPinAt(const Point& p) const
{
	const UnitPoint up = unit(p - m_offset);
	for (auto node : m_nodes)
	{
		Pin* pin = node->getPinAt(up);
		if (pin)
			return pin;
	}
	return nullptr;
}

void GraphControl::addDependencyHint(const Node* fromNode, const Node* toNode)
{
	if (fromNode == toNode)
		return;
	if (std::find_if(m_dependencyHints.begin(), m_dependencyHints.end(), [&](const std::pair< const Node*, const Node* >& v) {
		return v.first == fromNode && v.second == toNode;
	}) == m_dependencyHints.end())
		m_dependencyHints.push_back({ fromNode, toNode });
}

void GraphControl::removeDependencyHint(const Node* fromNode, const Node* toNode)
{
	auto it = std::find_if(m_dependencyHints.begin(), m_dependencyHints.end(), [&](const std::pair< const Node*, const Node* >& v) {
		return v.first == fromNode && v.second == toNode;
	});
	if (it != m_dependencyHints.end())
		m_dependencyHints.erase(it);
}

void GraphControl::removeAllDependencyHints()
{
	m_dependencyHints.resize(0);
}

void GraphControl::setPaintSettings(const PaintSettings& paintSettings)
{
	m_paintSettings = paintSettings;
}

const PaintSettings& GraphControl::getPaintSettings() const
{
	return m_paintSettings;
}

void GraphControl::setScale(float scale)
{
	m_scale = scale;
}

float GraphControl::getScale() const
{
	return m_scale;
}

void GraphControl::center(bool selectedOnly)
{
	RefArray< Node > nodes;
	if (!selectedOnly)
		nodes = m_nodes;
	else
		nodes = getSelectedNodes();
	if (nodes.empty())
		return;

	const Rect inner = getInnerRect();

	UnitRect bounds(
		Unit(std::numeric_limits< int32_t >::max()),
		Unit(std::numeric_limits< int32_t >::max()),
		Unit(-std::numeric_limits< int32_t >::max()),
		Unit(-std::numeric_limits< int32_t >::max())
	);
	for (auto node : nodes)
	{
		const UnitRect rc = node->calculateRect();
		bounds.left = std::min(bounds.left, rc.left);
		bounds.right = std::max(bounds.right, rc.right);
		bounds.top = std::min(bounds.top, rc.top);
		bounds.bottom = std::max(bounds.bottom, rc.bottom);
	}

	m_offset.cx = -(pixel(bounds.left + bounds.getWidth() / 2_ut) - (inner.getWidth() / m_scale) / 2);
	m_offset.cy = -(pixel(bounds.top + bounds.getHeight() / 2_ut) - (inner.getHeight() / m_scale) / 2);
}

void GraphControl::alignNodes(Alignment align)
{
	RefArray< Node > nodes = getSelectedNodes();

	UnitRect bounds(
		Unit(std::numeric_limits< int32_t >::max()),
		Unit(std::numeric_limits< int32_t >::max()),
		Unit(-std::numeric_limits< int32_t >::max()),
		Unit(-std::numeric_limits< int32_t >::max())
	);
	for (auto node : nodes)
	{
		const UnitRect rc = node->calculateRect();
		bounds.left = std::min(bounds.left, rc.left);
		bounds.right = std::max(bounds.right, rc.right);
		bounds.top = std::min(bounds.top, rc.top);
		bounds.bottom = std::max(bounds.bottom, rc.bottom);
	}

	for (auto node : nodes)
	{
		const UnitRect rc = node->calculateRect();
		UnitPoint pt = rc.getTopLeft();

		switch (align)
		{
		case AnLeft:
			pt.x = bounds.left;
			break;

		case AnTop:
			pt.y = bounds.top;
			break;

		case AnRight:
			pt.x = bounds.right - rc.getWidth();
			break;

		case AnBottom:
			pt.y = bounds.bottom - rc.getHeight();
			break;
		}

		if (pt != node->getPosition())
		{
			node->setPosition(pt);
			NodeMovedEvent event(this, node);
			raiseEvent(&event);
		}
	}
}

void GraphControl::evenSpace(EvenSpace space)
{
	RefArray< Node > nodes = getSelectedNodes();

	if (nodes.size() <= 1)
		return;

	nodes.sort(SortNodePred(space));

	UnitRect bounds(
		Unit(std::numeric_limits< int32_t >::max()),
		Unit(std::numeric_limits< int32_t >::max()),
		Unit(-std::numeric_limits< int32_t >::max()),
		Unit(-std::numeric_limits< int32_t >::max())
	);

	Unit totalWidth = 0_ut, totalHeight = 0_ut;

	for (auto node : nodes)
	{
		UnitRect rc = node->calculateRect();

		bounds.left = std::min(bounds.left, rc.left);
		bounds.right = std::max(bounds.right, rc.right);
		bounds.top = std::min(bounds.top, rc.top);
		bounds.bottom = std::max(bounds.bottom, rc.bottom);

		totalWidth += rc.getWidth();
		totalHeight += rc.getHeight();
	}

	const Unit spaceHoriz = (bounds.getWidth() - totalWidth) / Unit((int32_t)nodes.size() - 1);
	const Unit spaceVert = (bounds.getHeight() - totalHeight) / Unit((int32_t)nodes.size() - 1);

	Unit x = bounds.left, y = bounds.top;

	for (auto node : nodes)
	{
		const UnitRect rc = node->calculateRect();
		UnitPoint pt = rc.getTopLeft();

		switch (space)
		{
		case EsHorizontally:
			pt.x = x;
			break;

		case EsVertically:
			pt.y = y;
			break;
		}

		if (pt != node->getPosition())
		{
			node->setPosition(pt);
			NodeMovedEvent event(this, node);
			raiseEvent(&event);
		}

		x += rc.getWidth() + spaceHoriz;
		y += rc.getHeight() + spaceVert;
	}
}

Size GraphControl::getOffset() const
{
	return m_offset;
}

Point GraphControl::clientToVirtual(const Point& cpt) const
{
	return cpt / m_scale - m_offset;
}

Point GraphControl::virtualToClient(const Point& vpt) const
{
	return (vpt + m_offset) * m_scale;
}

Rect GraphControl::getVirtualRect() const
{
	const Rect rcClient = getInnerRect();
	return Rect(
		clientToVirtual(rcClient.getTopLeft()),
		clientToVirtual(rcClient.getBottomRight())
	);
}

void GraphControl::beginSelectModification()
{
	m_groupSelectionStates.resize(m_groups.size());
	for (size_t i = 0; i < m_groups.size(); ++i)
		m_groupSelectionStates[i] = m_groups[i]->isSelected();

	m_nodeSelectionStates.resize(m_nodes.size());
	for (size_t i = 0; i < m_nodes.size(); ++i)
		m_nodeSelectionStates[i] = m_nodes[i]->isSelected();

	m_edgeSelectionStates.resize(m_edges.size());
	for (size_t i = 0; i < m_edges.size(); ++i)
		m_edgeSelectionStates[i] = m_edges[i]->isSelected();
}

bool GraphControl::endSelectModification()
{
	RefArray< Group > groupSelectChanged;
	RefArray< Node > nodeSelectChanged;
	RefArray< Edge > edgeSelectChanged;

	T_ASSERT(m_groupSelectionStates.size() == m_groups.size());
	for (size_t i = 0; i < m_groups.size(); ++i)
	{
		if (m_groups[i]->isSelected() != m_groupSelectionStates[i])
			groupSelectChanged.push_back(m_groups[i]);
	}

	T_ASSERT(m_nodeSelectionStates.size() == m_nodes.size());
	for (size_t i = 0; i < m_nodes.size(); ++i)
	{
		if (m_nodes[i]->isSelected() != m_nodeSelectionStates[i])
			nodeSelectChanged.push_back(m_nodes[i]);
	}

	T_ASSERT(m_edgeSelectionStates.size() == m_edges.size());
	for (size_t i = 0; i < m_edges.size(); ++i)
	{
		if (m_edges[i]->isSelected() != m_edgeSelectionStates[i])
			edgeSelectChanged.push_back(m_edges[i]);
	}

	if (groupSelectChanged.empty() && nodeSelectChanged.empty() && edgeSelectChanged.empty())
		return false;

	SelectEvent selectEvent(this, groupSelectChanged, nodeSelectChanged, edgeSelectChanged);
	raiseEvent(&selectEvent);
	return true;
}

void GraphControl::capturePositions()
{
	m_groupPositions.resize(m_groups.size());
	m_groupAnchorPositions.resize(m_groups.size());
	for (uint32_t i = 0; i < m_groups.size(); ++i)
	{
		m_groupPositions[i] = m_groups[i]->calculateRect();
		if (m_groupAnchor >= 0)
			m_groupAnchorPositions[i] = m_groups[i]->getAnchorPosition(m_groupAnchor);
	}

	m_nodePositions.resize(m_nodes.size());
	for (uint32_t i = 0; i < m_nodes.size(); ++i)
		m_nodePositions[i] = m_nodes[i]->calculateRect();
}

void GraphControl::eventMouseDown(MouseButtonDownEvent* event)
{
	// Save positions of all groups and nodes so we can issue node moved events later,
	// we need to do this when we leave this method since order of nodes
	// might change.
	T_ANONYMOUS_VAR(Leave)([&]() {
		capturePositions();
	});

	if (!hasFocus())
		setFocus();

	// Save origin of drag and where the cursor is currently at.
	m_cursor =
	m_moveOrigin = event->getPosition() / m_scale;

	// If user holds down ALT we should move entire graph.
	if ((event->getKeyState() & KsMenu) != 0 || event->getButton() == MbtMiddle)
	{
		m_mode = MdMoveGraph;
		setCapture();
	}
	else if (event->getButton() == MbtLeft)
	{
		const Point hitPosition = m_cursor - m_offset;
		const UnitPoint unitHitPosition = unit(hitPosition);

		// Find top-most node, edge or group which contain mouse cursor.
		Ref< Group > selectedGroup;
		Ref< Edge > selectedEdge;
		Ref< Node > selectedNode;
		if (m_edgeSelectable)
			selectedEdge = getEdgeAt(hitPosition);
		if (!selectedEdge)
			selectedNode = getNodeAt(hitPosition);
		if (!selectedEdge && !selectedNode)
		{
			if ((selectedGroup = getGroupAt(hitPosition)) != nullptr)
			{
				m_groupAnchor = selectedGroup->hitAnchor(unitHitPosition);
				if (m_groupAnchor < 0)
				{
					// If not hitting an anchor we must ensure hit on group's title.
					if (!selectedGroup->hitTitle(unitHitPosition))
						selectedGroup = nullptr;
				}
			}
		}

		if (selectedNode)
		{
			beginSelectModification();

			// De-select all groups.
			for (auto group : m_groups)
				group->setSelected(false);

			// Update selection.
			if (!selectedNode->isSelected())
			{
				if (!(event->getKeyState() & KsShift))
				{
					// De-select all other nodes.
					for (auto node : m_nodes)
					{
						if (node != selectedNode)
							node->setSelected(false);
					}
				}
				selectedNode->setSelected(true);

				// Update edge selection states.
				for (auto edge : m_edges)
				{
					const bool selected =
						edge->getSourcePin()->getNode()->isSelected() ||
						edge->getDestinationPin()->getNode()->isSelected();

					edge->setSelected(selected);
				}
			}

			endSelectModification();

			// Ensure selected node is last.
			if (selectedNode != m_nodes.back())
			{
				m_nodes.remove(selectedNode);
				m_nodes.push_back(selectedNode);
			}

			// Check if an output pin was selected.
			if (event->getButton() == MbtLeft)
			{
				Ref< Pin > pin = selectedNode->getPinAt(unit(m_cursor - m_offset));
				if (pin)
				{
					if (pin->getDirection() == Pin::DrOutput)
						m_selectedPin = pin;
					else
					{
						if (m_mode != MdConnectEdge)
						{
							// See if we can find an existing edge connected to this input.
							for (Ref< Edge > edge : m_edges)
							{
								if (edge->getDestinationPin() != pin)
									continue;

								m_selectedPin = edge->getSourcePin();
								m_edges.remove(edge);

								EdgeDisconnectEvent edgeDisconnectEvent(this, edge);
								raiseEvent(&edgeDisconnectEvent);
								break;
							}
						}
						else if (pin != m_selectedPin)
						{
							// Connect edge.
							Ref< Edge > edge = new Edge(
								m_selectedPin,
								pin
							);

							EdgeConnectEvent edgeConnectEvent(this, edge);
							raiseEvent(&edgeConnectEvent);

							// Keep connecting edges if shift is being held.
							if ((event->getKeyState() & KsShift) == 0)
							{
								m_mode = MdNothing;
								m_selectedPin = nullptr;
								releaseCapture();
							}

							return;
						}
					}

					if (m_selectedPin)
					{
						// Adjust to the center of the pin.
						m_cursor = m_edgeOrigin = pixel(m_selectedPin->getPosition()) + m_offset;
						m_mode = MdDrawEdge;

						setCapture();
						return;
					}
				}

				// No pin selected, move the selected node(s).
				m_mode = MdMoveSelected;
				setCapture();
			}
		}
		else if (selectedEdge)
		{
			T_ASSERT(m_edgeSelectable);
			beginSelectModification();

			// De-select all groups.
			for (auto group : m_groups)
				group->setSelected(false);

			// Update selection.
			if (!selectedEdge->isSelected())
			{
				if (!(event->getKeyState() & KsShift))
				{
					// De-select all other edges.
					for (auto edge : m_edges)
					{
						if (edge != selectedEdge)
							edge->setSelected(false);
					}

					// De-select all nodes.
					for (auto node : m_nodes)
						node->setSelected(false);
				}
				selectedEdge->setSelected(true);
			}

			if (endSelectModification())
				update();

			m_mode = MdNothing;
		}
		else if (selectedGroup)
		{
			beginSelectModification();

			for (auto node : m_nodes)
				node->setSelected(false);

			for (auto edge : m_edges)
				edge->setSelected(false);

			for (auto group : m_groups)
				group->setSelected(group == selectedGroup);

			if (endSelectModification())
				update();

			m_mode = MdMoveSelected;
			setCapture();
		}
		else
		{
			if (!(event->getKeyState() & KsShift))
			{
				beginSelectModification();

				// De-select all groups and nodes, and then start drawing selection marker.
				for (auto group : m_groups)
					group->setSelected(false);
				for (auto node : m_nodes)
					node->setSelected(false);

				if (endSelectModification())
					update();
			}

			m_mode = MdDrawSelectionRectangle;
			setCapture();
		}
	}
}

void GraphControl::eventMouseUp(MouseButtonUpEvent* event)
{
	m_cursor = event->getPosition() / m_scale;

	if (m_mode == MdMoveSelected || m_mode == MdMoveGraph)
	{
		// Issue event for every group moved.
		T_ASSERT(m_groups.size() == m_groupPositions.size());
		for (uint32_t i = 0; i < m_groups.size(); ++i)
		{
			if (m_groups[i]->calculateRect() != m_groupPositions[i])
			{
				GroupMovedEvent event(this, m_groups[i]);
				raiseEvent(&event);
			}
		}

		// Issue event for every node moved.
		T_ASSERT(m_nodes.size() == m_nodePositions.size());
		for (uint32_t i = 0; i < m_nodes.size(); ++i)
		{
			if (m_nodes[i]->calculateRect() != m_nodePositions[i])
			{
				NodeMovedEvent event(this, m_nodes[i]);
				raiseEvent(&event);
			}
		}

		releaseCapture();
	}
	else
	{
		// Connect edge if we were drawing an edge.
		if (m_mode == MdDrawEdge)
		{
			Ref< Node > targetNode = getNodeAt(m_cursor - m_offset);
			if (targetNode)
			{
				Ref< Pin > targetPin = targetNode->getPinAt(unit(m_cursor - m_offset));
				if (targetPin)
				{
					if (targetPin->getDirection() == Pin::DrInput)
					{
						Ref< Edge > edge = new Edge(
							m_selectedPin,
							targetPin
						);
						EdgeConnectEvent event(this, edge);
						raiseEvent(&event);
					}
					else if (targetPin == m_selectedPin)
					{
						// Click on output pin; enter connect mode.
						m_cursor = m_edgeOrigin = pixel(m_selectedPin->getPosition()) + m_offset;
						m_mode = MdConnectEdge;
					}
				}
			}
		}

		// Select nodes which are within selection rectangle.
		else if (m_mode == MdDrawSelectionRectangle)
		{
			Point tl = m_moveOrigin;
			Point br = m_cursor;

			if (tl.x > br.x)
				std::swap(tl.x, br.x);
			if (tl.y > br.y)
				std::swap(tl.y, br.y);

			beginSelectModification();

			const Rect selection(tl, br);
			for (auto node : m_nodes)
			{
				const Rect rect = pixel(node->calculateRect()).offset(m_offset);
				if (selection.intersect(rect))
					node->setSelected(true);
			}

			// Update edge selection states.
			for (auto edge : m_edges)
			{
				const bool selected =
					edge->getSourcePin()->getNode()->isSelected() ||
					edge->getDestinationPin()->getNode()->isSelected();

				edge->setSelected(selected);
			}

			endSelectModification();
		}

		// Keep capture is we're connecting an edge.
		if (m_mode != MdConnectEdge)
		{
			m_mode = MdNothing;
			releaseCapture();
		}
	}

	m_mode = MdNothing;
	update();

	event->consume();
}

void GraphControl::eventMouseMove(MouseMoveEvent* event)
{
	if (m_mode == MdMoveGraph)
	{
		const Size delta = event->getPosition() / m_scale - m_moveOrigin;
		m_offset += delta;
		m_cursor += delta;
		m_edgeOrigin += delta;
		m_moveOrigin = event->getPosition() / m_scale;
		update();
		event->consume();
	}
	else if (m_mode == MdMoveSelected)
	{
		const Size offset = event->getPosition() / m_scale - m_moveOrigin;

		// Move selected groups.
		for (size_t i = 0; i < m_groups.size(); ++i)
		{
			Group* group = m_groups[i];
			if (!group->isSelected())
				continue;

			if (m_groupAnchor < 0)
			{
				const Point position = pixel(m_groupPositions[i].getTopLeft());
				group->setPosition(unit(position + offset));
			}
			else
			{
				const Point position = pixel(m_groupAnchorPositions[i]);
				group->setAnchorPosition(m_groupAnchor, unit(position + offset));
			}
		}

		// Move selected nodes.
		for (size_t i = 0; i < m_nodes.size(); ++i)
		{
			Node* node = m_nodes[i];
			if (!node->isSelected())
				continue;

			const Point position = pixel(m_nodePositions[i].getTopLeft());
			node->setPosition(unit(position + offset));
		}

		update();
		event->consume();
	}
	else if (m_mode == MdConnectEdge || m_mode == MdDrawEdge || m_mode == MdDrawSelectionRectangle)
	{
		Rect updateRect(
			m_moveOrigin * m_scale,
			m_cursor * m_scale
		);

		m_cursor = event->getPosition() / m_scale;
		updateRect = updateRect.getUnified().contain(m_cursor * m_scale).inflate(4, 4);

		update(&updateRect);
		event->consume();
	}

	const auto position = event->getPosition() / m_scale;

	// Track "hot" pin.
	auto hotPin = getPinAt(position);
	if (hotPin != m_hotPin)
	{
		m_hotPin = hotPin;
		update();
	}

	auto hotEdge = getEdgeAt(position);
	if (hotEdge != m_hotEdge)
	{
		m_hotEdge = hotEdge;
		update();
	}
}

void GraphControl::eventDoubleClick(MouseDoubleClickEvent* event)
{
	if (event->getButton() != MbtLeft)
		return;

	m_mode = MdNothing;

	// Issue activate on node only when a single node is selected.
	RefArray< Node > selectedNodes = getSelectedNodes();
	if (selectedNodes.size() == 1)
	{
		NodeActivateEvent activateEvent(this, selectedNodes.front());
		raiseEvent(&activateEvent);
	}
}

void GraphControl::eventMouseWheel(MouseWheelEvent* event)
{
	auto pos = screenToClient(event->getPosition());
	const Point p0 = clientToVirtual(pos);

	if (event->getRotation() < 0)
		m_scale *= 0.9f;
	else if (event->getRotation() > 0)
		m_scale /= 0.9f;

	const Point p1 = clientToVirtual(pos);
	m_offset += (p1 - p0);

	update();
	event->consume();
}

void GraphControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rc = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	// Select font from settings.
	canvas.setFont(m_paintSettings.getFont());

	// Draw grid.
	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rc);

	if (m_scale > 0.20f)
	{
		const int32_t gridSpacing = m_scale * pixel(32_ut);

		const int32_t ox = int32_t(m_offset.cx * m_scale) % gridSpacing;
		const int32_t oy = int32_t(m_offset.cy * m_scale) % gridSpacing;

		canvas.setForeground(ss->getColor(this, L"color-grid"));
		for (int32_t x = ox - gridSpacing; x < rc.getWidth(); x += gridSpacing)
			canvas.drawLine(x, rc.top, x, rc.bottom);

		for (int32_t y = oy - gridSpacing; y < rc.getHeight(); y += gridSpacing)
			canvas.drawLine(rc.left, y, rc.right, y);
	}

	// Draw text.
	const std::wstring text = getText();
	if (!text.empty())
	{
		auto fn = m_paintSettings.getFont();
		fn.setBold(true);
		fn.setSize(40_ut);
		canvas.setFont(fn);
		canvas.setForeground(ss->getColor(this, L"color-label"));
		canvas.drawText(rc.inflate(-pixel(8_ut), -pixel(8_ut)), text, ui::AnRight, ui::AnBottom);
		canvas.setFont(m_paintSettings.getFont());
	}

	// Draw arrow hints.
	canvas.setBackground(ss->getColor(this, L"color-arrow-hints"));
	const Point center = rc.getCenter();
	uint32_t arrowsDrawn = 0;
	for (auto node : m_nodes)
	{
		const Rect rcNode = pixel(node->calculateRect()).offset(m_offset);
		if ((arrowsDrawn & 1) == 0 && (int32_t)(rcNode.left * m_scale) < rc.left)
		{
			const Point p(rc.left + 16, center.y);
			const Point pl[] =
			{
				Point(p.x - 8, p.y),
				Point(p.x + 8, p.y - 8),
				Point(p.x + 8, p.y + 8)
			};
			canvas.fillPolygon(pl, 3);
			arrowsDrawn |= 1;
		}
		if ((arrowsDrawn & 2) == 0 && (int32_t)(rcNode.top * m_scale) < rc.top)
		{
			const Point p(center.x, rc.top + 16);
			const Point pl[] =
			{
				Point(p.x, p.y - 8),
				Point(p.x + 8, p.y + 8),
				Point(p.x - 8, p.y + 8)
			};
			canvas.fillPolygon(pl, 3);
			arrowsDrawn |= 2;
		}
		if ((arrowsDrawn & 4) == 0 && (int32_t)(rcNode.right * m_scale) > rc.right)
		{
			const Point p(rc.right - 16, center.y);
			const Point pl[] =
			{
				Point(p.x + 8, p.y),
				Point(p.x - 8, p.y - 8),
				Point(p.x - 8, p.y + 8)
			};
			canvas.fillPolygon(pl, 3);
			arrowsDrawn |= 4;
		}
		if ((arrowsDrawn & 8) == 0 && (int32_t)(rcNode.bottom * m_scale) > rc.bottom)
		{
			const Point p(center.x, rc.bottom - 16);
			const Point pl[] =
			{
				Point(p.x, p.y + 8),
				Point(p.x + 8, p.y - 8),
				Point(p.x - 8, p.y - 8)
			};
			canvas.fillPolygon(pl, 3);
			arrowsDrawn |= 8;
		}
		if (arrowsDrawn == (1 | 2 | 4 | 8))
			break;
	}

	GraphCanvas graphCanvas(
		this,
		&canvas,
		m_paintSettings,
		m_scale
	);

	// Draw groups.
	for (auto group : m_groups)
		group->paint(&graphCanvas, m_offset);

	// Draw dependency hints.
	for (const auto& hint : m_dependencyHints)
	{
		const Rect rcFrom = pixel(hint.first->calculateRect()).offset(m_offset);
		const Rect rcTo = pixel(hint.second->calculateRect()).offset(m_offset);
		graphCanvas.setForeground(Color4ub(100, 200, 100, 80));
		graphCanvas.drawLine(rcFrom.getCenter(), rcTo.getCenter(), 4);
	}

	// Draw edges.
	for (auto edge : m_edges)
	{
		if (!edge->isSelected() && m_hotEdge != edge)
			edge->paint(this, &graphCanvas, m_offset, m_imageLabel, false);
	}

	// Node shapes.
	graphCanvas.setFont(m_paintSettings.getFont());
	const Rect cullRc = rc / m_scale;
	for (auto node : m_nodes)
	{
		if (pixel(node->calculateRect()).offset(m_offset).intersect(cullRc))
			node->paint(&graphCanvas, m_hotPin, m_offset);

#if defined(_DEBUG)
		graphCanvas.setForeground(Color4ub(255, 255, 255, 255));
		graphCanvas.drawRect(
			pixel(node->calculateRect()).offset(m_offset)
		);
#endif
	}

	// Draw selected, or hot, edges.
	for (auto edge : m_edges)
	{
		if (edge->isSelected() || m_hotEdge == edge)
			edge->paint(this, &graphCanvas, m_offset, m_imageLabel, (bool)(m_hotEdge == edge));
	}

	// Edge cursor.
	if (m_mode == MdConnectEdge || m_mode == MdDrawEdge)
	{
		graphCanvas.setForeground(ss->getColor(this, L"color-edge-hint"));
		graphCanvas.drawLine(m_edgeOrigin, m_cursor, 1);
	}

	// Selection rectangle.
	if (m_mode == MdDrawSelectionRectangle)
	{
		graphCanvas.setForeground(Color4ub(220, 220, 255, 200));
		graphCanvas.setBackground(Color4ub(90, 90, 120, 80));
		graphCanvas.fillRect(Rect(m_moveOrigin, m_cursor));
		graphCanvas.drawRect(Rect(m_moveOrigin, m_cursor));
	}

	event->consume();
}

void GraphControl::eventSize(SizeEvent* event)
{
	for (auto node : m_nodes)
		node->updateSize();
	update();
	event->consume();
}

}
