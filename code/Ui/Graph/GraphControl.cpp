/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <limits>
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/GraphCanvas.h"
#include "Ui/Graph/PaintSettings.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/NodeActivateEvent.h"
#include "Ui/Graph/NodeMovedEvent.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/EdgeConnectEvent.h"
#include "Ui/Graph/EdgeDisconnectEvent.h"
#include "Ui/Graph/Pin.h"
#include "Ui/Graph/SelectEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

enum Modes
{
	MdNothing,
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
		Point pt1 = n1->calculateRect().getTopLeft();
		Point pt2 = n2->calculateRect().getTopLeft();
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
,	m_moveAll(false)
,	m_moveSelected(false)
,	m_edgeSelectable(false)
,	m_hotPin(nullptr)
{
}

bool GraphControl::create(Widget* parent, int style)
{
	if (!Widget::create(parent, style))
		return false;

	m_paintSettings = new PaintSettings(getFont());
	m_imageLabel = new ui::StyleBitmap(L"UI.Graph.Label");

	addEventHandler< MouseButtonDownEvent >(this, &GraphControl::eventMouseDown);
	addEventHandler< MouseButtonUpEvent >(this, &GraphControl::eventMouseUp);
	addEventHandler< MouseMoveEvent >(this, &GraphControl::eventMouseMove);
	addEventHandler< MouseDoubleClickEvent >(this, &GraphControl::eventDoubleClick);
	addEventHandler< MouseWheelEvent >(this, &GraphControl::eventMouseWheel);
	addEventHandler< PaintEvent >(this, &GraphControl::eventPaint);

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

Node* GraphControl::createNode(const std::wstring& title, const std::wstring& info, const Point& position, const INodeShape* shape)
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
}

void GraphControl::removeAllNodes()
{
	for (auto node : m_nodes)
		node->m_owner = nullptr;

	m_nodes.resize(0);
	m_edges.resize(0);
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

int GraphControl::getSelectedNodes(RefArray< Node >& out) const
{
	out.resize(0);
	for (auto node : m_nodes)
	{
		if (node->isSelected())
			out.push_back(node);
	}
	return (int)out.size();
}

int GraphControl::getSelectedEdges(RefArray< Edge >& out) const
{
	out.resize(0);
	for (auto edge : m_edges)
	{
		if (edge->isSelected())
			out.push_back(edge);
	}
	return (int)out.size();
}

int GraphControl::getConnectedEdges(const Pin* pin, RefArray< Edge >& outEdges) const
{
	outEdges.resize(0);
	for (auto edge : m_edges)
	{
		if (edge->getSourcePin() == pin || edge->getDestinationPin() == pin)
			outEdges.push_back(edge);
	}
	return (int)outEdges.size();
}

int GraphControl::getConnectedEdges(const Node* node, RefArray< Edge >& outEdges) const
{
	outEdges.resize(0);
	for (auto edge : m_edges)
	{
		if (edge->getSourcePin()->getNode() == node || edge->getDestinationPin()->getNode() == node)
			outEdges.push_back(edge);
	}
	return (int)outEdges.size();
}

int GraphControl::getConnectedEdges(const RefArray< Node >& nodes, bool inclusive, RefArray< Edge >& outEdges) const
{
	outEdges.resize(0);
	for (auto edge : m_edges)
	{
		bool n1 = bool(std::find(nodes.begin(), nodes.end(), edge->getSourcePin()->getNode()) != nodes.end());
		bool n2 = bool(std::find(nodes.begin(), nodes.end(), edge->getDestinationPin()->getNode()) != nodes.end());
		if ((inclusive && (n1 && n2)) || (!inclusive && (n1 || n2)))
			outEdges.push_back(edge);
	}
	return (int)outEdges.size();
}

Node* GraphControl::getNodeAt(const Point& p) const
{
	for (auto node : m_nodes)
	{
		if (node->hit(p))
			return node;
	}
	return nullptr;
}

Edge* GraphControl::getEdgeAt(const Point& p) const
{
	for (auto edge : m_edges)
	{
		if (edge->hit(p))
			return edge;
	}
	return nullptr;
}

Pin* GraphControl::getPinAt(const Point& p) const
{
	for (auto node : m_nodes)
	{
		Pin* pin = node->getPinAt(p - m_offset);
		if (pin)
			return pin;
	}
	return nullptr;
}

void GraphControl::setPaintSettings(const PaintSettings* paintSettings)
{
	m_paintSettings = paintSettings;
}

const PaintSettings* GraphControl::getPaintSettings() const
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
		getSelectedNodes(nodes);
	if (nodes.empty())
		return;

	Rect inner = getInnerRect();

	Rect bounds(
		std::numeric_limits< int >::max(),
		std::numeric_limits< int >::max(),
		-std::numeric_limits< int >::max(),
		-std::numeric_limits< int >::max()
	);
	for (auto node : nodes)
	{
		Rect rc = node->calculateRect();
		bounds.left = std::min(bounds.left, rc.left);
		bounds.right = std::max(bounds.right, rc.right);
		bounds.top = std::min(bounds.top, rc.top);
		bounds.bottom = std::max(bounds.bottom, rc.bottom);
	}

	m_offset.cx = -(int32_t)(bounds.left + bounds.getWidth() / 2 - (inner.getWidth() / m_scale) / 2);
	m_offset.cy = -(int32_t)(bounds.top + bounds.getHeight() / 2 - (inner.getHeight() / m_scale) / 2);
}

void GraphControl::alignNodes(Alignment align)
{
	RefArray< Node > nodes;
	getSelectedNodes(nodes);

	Rect bounds(
		std::numeric_limits< int32_t >::max(),
		std::numeric_limits< int32_t >::max(),
		-std::numeric_limits< int32_t >::max(),
		-std::numeric_limits< int32_t >::max()
	);
	for (auto node : nodes)
	{
		Rect rc = node->calculateRect();
		bounds.left = std::min(bounds.left, rc.left);
		bounds.right = std::max(bounds.right, rc.right);
		bounds.top = std::min(bounds.top, rc.top);
		bounds.bottom = std::max(bounds.bottom, rc.bottom);
	}

	for (auto node : nodes)
	{
		Rect rc = node->calculateRect();
		Point pt = rc.getTopLeft();

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
	RefArray< Node > nodes;
	getSelectedNodes(nodes);

	if (nodes.size() <= 1)
		return;

	nodes.sort(SortNodePred(space));

	Rect bounds(
		std::numeric_limits< int32_t >::max(),
		std::numeric_limits< int32_t >::max(),
		-std::numeric_limits< int32_t >::max(),
		-std::numeric_limits< int32_t >::max()
	);

	int32_t totalWidth = 0, totalHeight = 0;

	for (auto node : nodes)
	{
		Rect rc = node->calculateRect();

		bounds.left = std::min(bounds.left, rc.left);
		bounds.right = std::max(bounds.right, rc.right);
		bounds.top = std::min(bounds.top, rc.top);
		bounds.bottom = std::max(bounds.bottom, rc.bottom);

		totalWidth += rc.getWidth();
		totalHeight += rc.getHeight();
	}

	int32_t spaceHoriz = (bounds.getWidth() - totalWidth) / (int32_t)(nodes.size() - 1);
	int32_t spaceVert = (bounds.getHeight() - totalHeight) / (int32_t)(nodes.size() - 1);

	int32_t x = bounds.left, y = bounds.top;

	for (auto node : nodes)
	{
		Rect rc = node->calculateRect();
		Point pt = rc.getTopLeft();

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
	Rect rcClient = getInnerRect();
	return Rect(
		clientToVirtual(rcClient.getTopLeft()),
		clientToVirtual(rcClient.getBottomRight())
	);
}

void GraphControl::beginSelectModification()
{
	m_nodeSelectionStates.resize(m_nodes.size());
	for (size_t i = 0; i < m_nodes.size(); ++i)
		m_nodeSelectionStates[i] = m_nodes[i]->isSelected();

	m_edgeSelectionStates.resize(m_edges.size());
	for (size_t i = 0; i < m_edges.size(); ++i)
		m_edgeSelectionStates[i] = m_edges[i]->isSelected();
}

bool GraphControl::endSelectModification()
{
	RefArray< Node > nodeSelectChanged;
	RefArray< Edge > edgeSelectChanged;

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

	if (nodeSelectChanged.empty() && edgeSelectChanged.empty())
		return false;

	SelectEvent selectEvent(this, nodeSelectChanged, edgeSelectChanged);
	raiseEvent(&selectEvent);
	return true;
}

void GraphControl::eventMouseDown(MouseButtonDownEvent* event)
{
	setFocus();

	m_moveAll = false;
	m_moveSelected = false;

	m_selectedEdge = nullptr;
	m_selectedNode = nullptr;

	// Save origin of drag and where the cursor is currently at.
	m_cursor =
	m_moveOrigin = event->getPosition() / m_scale;

	// Save positions of all nodes so we can issue node moved events later..
	m_nodePositions.resize(m_nodes.size());
	for (uint32_t i = 0; i < m_nodes.size(); ++i)
		m_nodePositions[i] = m_nodes[i]->getPosition();

	// If user holds down ALT we should move entire graph.
	if ((event->getKeyState() & KsMenu) != 0 || event->getButton() == MbtMiddle)
	{
		m_moveAll = true;
		setCapture();
		return;
	}

	// Find top-most node or edge which contain mouse cursor.
	if (m_edgeSelectable)
		m_selectedEdge = getEdgeAt(m_cursor - m_offset);
	if (!m_selectedEdge)
		m_selectedNode = getNodeAt(m_cursor - m_offset);

	if (m_selectedNode && !m_selectedEdge)
	{
		beginSelectModification();

		// Update selection.
		if (!m_selectedNode->isSelected())
		{
			if (!(event->getKeyState() & KsShift))
			{
				// Deselect all other nodes.
				for (auto node : m_nodes)
				{
					if (node != m_selectedNode)
						node->setSelected(false);
				}
			}
			m_selectedNode->setSelected(true);

			// Update edge selection states.
			for (auto edge : m_edges)
			{
				bool selected =
					edge->getSourcePin()->getNode()->isSelected() ||
					edge->getDestinationPin()->getNode()->isSelected();

				edge->setSelected(selected);
			}
		}

		endSelectModification();

		// Ensure selected node is last.
		if (m_selectedNode != m_nodes.back())
		{
			m_nodes.remove(m_selectedNode);
			m_nodes.push_back(m_selectedNode);
		}

		// Check if an output pin was selected.
		if (event->getButton() == MbtLeft)
		{
			Ref< Pin > pin = m_selectedNode->getPinAt(m_cursor - m_offset);
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
					m_cursor = m_edgeOrigin = m_selectedPin->getPosition() + m_offset;
					m_mode = MdDrawEdge;

					setCapture();
					return;
				}
			}

			// No pin selected, move the selected node(s).
			m_moveSelected = true;
			setCapture();
		}
	}
	else if (!m_selectedNode && m_selectedEdge)
	{
		T_ASSERT(m_edgeSelectable);
		beginSelectModification();

		// Update selection.
		if (!m_selectedEdge->isSelected())
		{
			if (!(event->getKeyState() & KsShift))
			{
				// Deselect all other edges.
				for (auto edge : m_edges)
				{
					if (edge != m_selectedEdge)
						edge->setSelected(false);
				}

				// Deselect all nodes.
				for (auto node : m_nodes)
					node->setSelected(false);
			}
			m_selectedEdge->setSelected(true);
		}

		if (endSelectModification())
			update();

		m_mode = MdNothing;
	}
	else
	{
		// Selection or move must be made by left mouse button.
		if (event->getButton() != MbtLeft)
			return;

		if (!(event->getKeyState() & KsShift))
		{
			beginSelectModification();

			// Deselect all nodes and start drawing selection marker.
			for (auto node : m_nodes)
				node->setSelected(false);

			if (endSelectModification())
				update();
		}

		m_mode = MdDrawSelectionRectangle;

		setCapture();
	}
}

void GraphControl::eventMouseUp(MouseButtonUpEvent* event)
{
	m_cursor = event->getPosition() / m_scale;

	if (m_moveAll || m_moveSelected)
	{
		T_ASSERT(m_nodes.size() == m_nodePositions.size());
		for (uint32_t i = 0; i < m_nodes.size(); ++i)
		{
			if (m_nodes[i]->getPosition() != m_nodePositions[i])
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
				Ref< Pin > targetPin = targetNode->getPinAt(m_cursor - m_offset);
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
						m_cursor = m_edgeOrigin = m_selectedPin->getPosition() + m_offset;
						m_mode = MdConnectEdge;
					}
				}
			}
		}

		// Select nodes which are within selection rectangle.
		if (m_mode == MdDrawSelectionRectangle)
		{
			Point tl = m_moveOrigin;
			Point br = m_cursor;

			if (tl.x > br.x)
				std::swap(tl.x, br.x);
			if (tl.y > br.y)
				std::swap(tl.y, br.y);

			beginSelectModification();

			Rect selection(tl, br);
			for (auto node : m_nodes)
			{
				Rect rect = node->calculateRect().offset(m_offset);
				if (selection.intersect(rect))
					node->setSelected(true);
			}

			// Update edge selection states.
			for (auto edge : m_edges)
			{
				bool selected =
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

	m_moveAll = false;
	m_moveSelected = false;

	update();

	event->consume();
}

void GraphControl::eventMouseMove(MouseMoveEvent* event)
{
	if (m_moveAll)
	{
		Size delta = event->getPosition() / m_scale - m_moveOrigin;
		m_offset += delta;
		m_cursor += delta;
		m_edgeOrigin += delta;
		m_moveOrigin = event->getPosition() / m_scale;
		update();
		event->consume();
	}
	else if (m_moveSelected)
	{
		Size offset = event->getPosition() / m_scale - m_moveOrigin;
		for (auto node : m_nodes)
		{
			if (!node->isSelected())
				continue;

			Point position = node->getPosition();
			node->setPosition(position + offset);
		}

		m_moveOrigin = event->getPosition() / m_scale;
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

	// Track "hot" pin.
	auto position = event->getPosition() / m_scale;
	auto hotPin = getPinAt(position);
	if (hotPin != m_hotPin)
	{
		m_hotPin = hotPin;
		update();
	}
}

void GraphControl::eventDoubleClick(MouseDoubleClickEvent* event)
{
	if (event->getButton() != MbtLeft || !m_selectedNode)
		return;

	m_moveAll = false;
	m_moveSelected = false;

	NodeActivateEvent activateEvent(this, m_selectedNode);
	raiseEvent(&activateEvent);

	event->consume();
}

void GraphControl::eventMouseWheel(MouseWheelEvent* event)
{
	auto pos = screenToClient(event->getPosition());
	Point p0 = clientToVirtual(pos);

	if (event->getRotation() < 0)
		m_scale *= 0.9f;
	else if (event->getRotation() > 0)
		m_scale /= 0.9f;

	Point p1 = clientToVirtual(pos);
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
	canvas.setFont(m_paintSettings->getFont());

	// Draw grid.
	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rc);

	const int32_t gridSpacing = m_scale * dpi96(32);

	const int32_t ox = int32_t(m_offset.cx * m_scale) % gridSpacing;
	const int32_t oy = int32_t(m_offset.cy * m_scale) % gridSpacing;

	canvas.setForeground(ss->getColor(this, L"color-grid"));
	for (int32_t x = ox - gridSpacing; x < rc.getWidth(); x += gridSpacing)
		canvas.drawLine(x, rc.top, x, rc.bottom);

	for (int32_t y = oy - gridSpacing; y < rc.getHeight(); y += gridSpacing)
		canvas.drawLine(rc.left, y, rc.right, y);

	// Draw text.
	const std::wstring text = getText();
	if (!text.empty())
	{
		auto fn = m_paintSettings->getFont();
		fn.setSize(40);
		canvas.setFont(fn);
		canvas.setForeground(ss->getColor(this, L"color-label"));
		canvas.drawText(rc.inflate(-dpi96(8), -dpi96(8)), text, ui::AnRight, ui::AnBottom);
		canvas.setFont(m_paintSettings->getFont());
	}

	// Draw arrow hints.
	canvas.setBackground(ss->getColor(this, L"color-arrow-hints"));
	const Point center = rc.getCenter();
	uint32_t arrowsDrawn = 0;
	for (auto node : m_nodes)
	{
		Rect rcNode = node->calculateRect().offset(m_offset);
		if ((arrowsDrawn & 1) == 0 && (int32_t)(rcNode.left * m_scale) < rc.left)
		{
			Point p(rc.left + 16, center.y);
			Point pl[] =
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
			Point p(center.x, rc.top + 16);
			Point pl[] =
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
			Point p(rc.right - 16, center.y);
			Point pl[] =
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
			Point p(center.x, rc.bottom - 16);
			Point pl[] =
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
		&canvas,
		m_paintSettings,
		m_scale
	);

	// Draw edges.
	for (auto edge : m_edges)
	{
		if (!edge->isSelected())
			edge->paint(this, &graphCanvas, m_offset, m_imageLabel);
	}

	// Node shapes.
	graphCanvas.setFont(m_paintSettings->getFont());
	Rect cullRc = rc / m_scale;
	for (auto node : m_nodes)
	{
		if (node->calculateRect().offset(m_offset).intersect(cullRc))
			node->paint(&graphCanvas, m_hotPin, m_offset);

#if defined(_DEBUG)
		graphCanvas.setForeground(Color4ub(255, 255, 255, 255));
		graphCanvas.drawRect(
			node->calculateRect().offset(m_offset)
		);
#endif
	}

	// Draw selected edges.
	for (auto edge : m_edges)
	{
		if (edge->isSelected())
			edge->paint(this, &graphCanvas, m_offset, m_imageLabel);
	}

	// Edge cursor.
	if (m_mode == MdConnectEdge || m_mode == MdDrawEdge)
	{
		graphCanvas.setForeground(ss->getColor(this, L"color-edge-hint"));
		graphCanvas.drawLine(m_edgeOrigin, m_cursor);
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

	}
}
