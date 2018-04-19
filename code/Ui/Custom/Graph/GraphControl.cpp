/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <limits>
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/PaintSettings.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/NodeActivateEvent.h"
#include "Ui/Custom/Graph/NodeMovedEvent.h"
#include "Ui/Custom/Graph/Edge.h"
#include "Ui/Custom/Graph/EdgeConnectEvent.h"
#include "Ui/Custom/Graph/EdgeDisconnectEvent.h"
#include "Ui/Custom/Graph/Pin.h"
#include "Ui/Custom/Graph/SelectEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

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

			}

enum Modes
{
	MdNothing,
	MdDrawEdge,
	MdConnectEdge,
	MdDrawSelectionRectangle
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GraphControl", GraphControl, Widget)

GraphControl::GraphControl()
:	m_mode(MdNothing)
,	m_moveAll(false)
,	m_moveSelected(false)
,	m_edgeSelectable(false)
{
}

bool GraphControl::create(Widget* parent, int style)
{
	if (!Widget::create(parent, style))
		return false;

	m_paintSettings = new PaintSettings(getFont());
	m_imageBackground = new ui::StyleBitmap(L"UI.Graph.Background");

	addEventHandler< MouseButtonDownEvent >(this, &GraphControl::eventMouseDown);
	addEventHandler< MouseButtonUpEvent >(this, &GraphControl::eventMouseUp);
	addEventHandler< MouseMoveEvent >(this, &GraphControl::eventMouseMove);
	addEventHandler< MouseDoubleClickEvent >(this, &GraphControl::eventDoubleClick);
	addEventHandler< PaintEvent >(this, &GraphControl::eventPaint);

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
	m_nodes.clear();
	m_edges.clear();
	Widget::destroy();
}

void GraphControl::addNode(Node* node)
{
	m_nodes.insert(m_nodes.begin(), node);
}

void GraphControl::removeNode(Node* node)
{
	RefArray< Node >::iterator i = std::find(m_nodes.begin(), m_nodes.end(), node);
	m_nodes.erase(i);
}

void GraphControl::removeAllNodes()
{
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
	for (RefArray< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
		(*i)->setSelected(true);
}

void GraphControl::deselectAllNodes()
{
	for (RefArray< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
		(*i)->setSelected(false);
}

void GraphControl::addEdge(Edge* edge)
{
	m_edges.push_back(edge);
}

void GraphControl::removeEdge(Edge* edge)
{
	RefArray< Edge >::iterator i = std::find(m_edges.begin(), m_edges.end(), edge);
	m_edges.erase(i);
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
	for (RefArray< Node >::const_iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
	{
		if ((*i)->isSelected())
			out.push_back(*i);
	}
	return int(out.size());
}

int GraphControl::getSelectedEdges(RefArray< Edge >& out) const
{
	for (RefArray< Edge >::const_iterator i = m_edges.begin(); i != m_edges.end(); ++i)
	{
		if ((*i)->isSelected())
			out.push_back(*i);
	}
	return int(out.size());
}

int GraphControl::getConnectedEdges(const Pin* pin, RefArray< Edge >& outEdges) const
{
	for (RefArray< Edge >::const_iterator i = m_edges.begin(); i != m_edges.end(); ++i)
	{
		if ((*i)->getSourcePin() == pin || (*i)->getDestinationPin() == pin)
			outEdges.push_back(*i);
	}
	return int(outEdges.size());
}

int GraphControl::getConnectedEdges(const Node* node, RefArray< Edge >& outEdges) const
{
	for (RefArray< Edge >::const_iterator i = m_edges.begin(); i != m_edges.end(); ++i)
	{
		if ((*i)->getSourcePin()->getNode() == node || (*i)->getDestinationPin()->getNode() == node)
			outEdges.push_back(*i);
	}
	return int(outEdges.size());
}

int GraphControl::getConnectedEdges(const RefArray< Node >& nodes, bool inclusive, RefArray< Edge >& outEdges) const
{
	for (RefArray< Edge >::const_iterator i = m_edges.begin(); i != m_edges.end(); ++i)
	{
		bool n1 = bool(std::find(nodes.begin(), nodes.end(), (*i)->getSourcePin()->getNode()) != nodes.end());
		bool n2 = bool(std::find(nodes.begin(), nodes.end(), (*i)->getDestinationPin()->getNode()) != nodes.end());

		if ((inclusive && (n1 && n2)) || (!inclusive && (n1 || n2)))
			outEdges.push_back(*i);
	}
	return int(outEdges.size());
}

Node* GraphControl::getNodeAt(const Point& p) const
{
	for (RefArray< Node >::const_iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
	{
		if ((*i)->hit(p))
			return (*i);
	}
	return 0;
}

Edge* GraphControl::getEdgeAt(const Point& p) const
{
	for (RefArray< Edge >::const_iterator i = m_edges.begin(); i != m_edges.end(); ++i)
	{
		if ((*i)->hit(m_paintSettings, p))
			return (*i);
	}
	return 0;
}

Pin* GraphControl::getPinAt(const Point& p) const
{
	for (RefArray< Node >::const_iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
	{
		Pin* pin = (*i)->getPinAt(p - m_offset);
		if (pin)
			return pin;
	}
	return 0;
}

void GraphControl::showProbe(const Point& p, const std::wstring& text)
{
	m_probeAt = p;
	m_probeText = text;
	update();
}

void GraphControl::hideProbe()
{
	if (!m_probeText.empty())
	{
		m_probeText.clear();
		update();
	}
}

void GraphControl::setPaintSettings(const PaintSettings* paintSettings)
{
	m_paintSettings = paintSettings;
}

const PaintSettings* GraphControl::getPaintSettings() const
{
	return m_paintSettings;
}

void GraphControl::center()
{
	if (m_nodes.empty())
		return;

	Rect inner = getInnerRect();

	Rect bounds(
		std::numeric_limits< int >::max(),
		std::numeric_limits< int >::max(),
		-std::numeric_limits< int >::max(),
		-std::numeric_limits< int >::max()
	);
	for (RefArray< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
	{
		Rect rc = (*i)->calculateRect();
		bounds.left = std::min(bounds.left, rc.left);
		bounds.right = std::max(bounds.right, rc.right);
		bounds.top = std::min(bounds.top, rc.top);
		bounds.bottom = std::max(bounds.bottom, rc.bottom);
	}

	m_offset.cx = -(bounds.left + bounds.getWidth() / 2 - inner.getWidth() / 2);
	m_offset.cy = -(bounds.top + bounds.getHeight() / 2 - inner.getHeight() / 2);
}

void GraphControl::alignNodes(Alignment align)
{
	RefArray< Node > nodes;
	getSelectedNodes(nodes);

	Rect bounds(
		std::numeric_limits< int >::max(),
		std::numeric_limits< int >::max(),
		-std::numeric_limits< int >::max(),
		-std::numeric_limits< int >::max()
	);
	for (RefArray< Node >::iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		Rect rc = (*i)->calculateRect();
		bounds.left = std::min(bounds.left, rc.left);
		bounds.right = std::max(bounds.right, rc.right);
		bounds.top = std::min(bounds.top, rc.top);
		bounds.bottom = std::max(bounds.bottom, rc.bottom);
	}

	for (RefArray< Node >::iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		Rect rc = (*i)->calculateRect();
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

		if (pt != (*i)->getPosition())
		{
			(*i)->setPosition(pt);

			NodeMovedEvent event(this, *i);
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
		std::numeric_limits< int >::max(),
		std::numeric_limits< int >::max(),
		-std::numeric_limits< int >::max(),
		-std::numeric_limits< int >::max()
	);

	int totalWidth = 0, totalHeight = 0;

	for (RefArray< Node >::iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		Rect rc = (*i)->calculateRect();

		bounds.left = std::min(bounds.left, rc.left);
		bounds.right = std::max(bounds.right, rc.right);
		bounds.top = std::min(bounds.top, rc.top);
		bounds.bottom = std::max(bounds.bottom, rc.bottom);

		totalWidth += rc.getWidth();
		totalHeight += rc.getHeight();
	}

	int spaceHoriz = (bounds.getWidth() - totalWidth) / int(nodes.size() - 1);
	int spaceVert = (bounds.getHeight() - totalHeight) / int(nodes.size() - 1);

	int x = bounds.left, y = bounds.top;

	for (RefArray< Node >::iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		Rect rc = (*i)->calculateRect();
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

		if (pt != (*i)->getPosition())
		{
			(*i)->setPosition(pt);

			NodeMovedEvent event(this, *i);
			raiseEvent(&event);
		}

		x += rc.getWidth() + spaceHoriz;
		y += rc.getHeight() + spaceVert;
	}
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

	T_ASSERT (m_nodeSelectionStates.size() == m_nodes.size());
	for (size_t i = 0; i < m_nodes.size(); ++i)
	{
		if (m_nodes[i]->isSelected() != m_nodeSelectionStates[i])
			nodeSelectChanged.push_back(m_nodes[i]);
	}

	T_ASSERT (m_edgeSelectionStates.size() == m_edges.size());
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

	m_selectedEdge = 0;
	m_selectedNode = 0;

	// Save origin of drag and where the cursor is currently at.
	m_cursor =
	m_moveOrigin = event->getPosition();

	// Save positions of all nodes so we can issue node moved events later..
	m_nodePositions.resize(m_nodes.size());
	for (uint32_t i = 0; i < m_nodes.size(); ++i)
		m_nodePositions[i] = m_nodes[i]->getPosition();

	// If user holds down ALT we should move entire graph.
	if ((event->getKeyState() & KsMenu) != 0)
	{
		m_moveAll = true;
		setCapture();
		event->consume();
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
				for (RefArray< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
				{
					if ((*i) != m_selectedNode)
						(*i)->setSelected(false);
				}
			}
			m_selectedNode->setSelected(true);

			// Update edge selection states.
			for (RefArray< Edge >::iterator i = m_edges.begin(); i != m_edges.end(); ++i)
			{
				bool selected =
					(*i)->getSourcePin()->getNode()->isSelected() ||
					(*i)->getDestinationPin()->getNode()->isSelected();

				(*i)->setSelected(selected);
			}

			// Move selected node last in list to ensure it's drawn last.
			RefArray< Node >::iterator i = std::find(m_nodes.begin(), m_nodes.end(), m_selectedNode);
			m_nodes.erase(i);
			m_nodes.push_back(m_selectedNode);
		}

		endSelectModification();

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
						for (RefArray< Edge >::iterator i = m_edges.begin(); i != m_edges.end(); ++i)
						{
							Ref< Edge > edge = *i;
							if (edge->getDestinationPin() != pin)
								continue;

							m_selectedPin = edge->getSourcePin();
							m_edges.erase(i);

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
							m_selectedPin = 0;
							releaseCapture();
						}

						event->consume();
						return;
					}
				}

				if (m_selectedPin)
				{
					// Adjust to the center of the pin.
					m_cursor = m_edgeOrigin = m_selectedPin->getPosition() + m_offset;
					m_mode = MdDrawEdge;

					setCapture();
					event->consume();
					return;
				}
			}

			// No pin selected, move the selected node(s).
			m_moveSelected = true;
			setCapture();
			event->consume();
		}
	}
	else if (!m_selectedNode && m_selectedEdge)
	{
		T_ASSERT (m_edgeSelectable);
		beginSelectModification();

		// Update selection.
		if (!m_selectedEdge->isSelected())
		{
			if (!(event->getKeyState() & KsShift))
			{
				// Deselect all other edges.
				for (RefArray< Edge >::iterator i = m_edges.begin(); i != m_edges.end(); ++i)
				{
					if ((*i) != m_selectedEdge)
						(*i)->setSelected(false);
				}

				// Deselect all nodes.
				for (RefArray< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
					(*i)->setSelected(false);
			}
			m_selectedEdge->setSelected(true);
		}

		if (endSelectModification())
			update();

		m_mode = MdNothing;
		event->consume();
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
			for (RefArray< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
				(*i)->setSelected(false);

			if (endSelectModification())
				update();
		}

		m_mode = MdDrawSelectionRectangle;

		setCapture();
		event->consume();
	}
}

void GraphControl::eventMouseUp(MouseButtonUpEvent* event)
{
	m_cursor = event->getPosition();

	if (m_moveAll || m_moveSelected)
	{
		T_ASSERT (m_nodes.size() == m_nodePositions.size());
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

			Rect selection = Rect(tl, br);
			for(RefArray< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
			{
				Rect rect = (*i)->calculateRect().offset(m_offset);
				if (selection.intersect(rect))
					(*i)->setSelected(true);
			}

			// Update edge selection states.
			for (RefArray< Edge >::iterator i = m_edges.begin(); i != m_edges.end(); ++i)
			{
				bool selected =
					(*i)->getSourcePin()->getNode()->isSelected() ||
					(*i)->getDestinationPin()->getNode()->isSelected();

				(*i)->setSelected(selected);
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
		Size delta = event->getPosition() - m_moveOrigin;
		m_offset += delta;
		m_cursor += delta;
		m_edgeOrigin += delta;
		m_moveOrigin = event->getPosition();
		update();
		event->consume();
	}
	else if (m_moveSelected)
	{
		Size offset = event->getPosition() - m_moveOrigin;
		for (RefArray< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
		{
			if (!(*i)->isSelected())
				continue;

			Point position = (*i)->getPosition();
			(*i)->setPosition(position + offset);
		}

		m_moveOrigin = event->getPosition();
		update();
		event->consume();
	}
	else if (m_mode == MdConnectEdge || m_mode == MdDrawEdge || m_mode == MdDrawSelectionRectangle)
	{
		Rect updateRect(
			m_moveOrigin,
			m_cursor
		);

		m_cursor = event->getPosition();
		updateRect = updateRect.getUnified().contain(m_cursor).inflate(4, 4);

		update(&updateRect);
		event->consume();
	}
}

void GraphControl::eventDoubleClick(MouseDoubleClickEvent* event)
{
	if (event->getButton() != MbtLeft || !m_selectedNode)
		return;

	NodeActivateEvent activateEvent(this, m_selectedNode);
	raiseEvent(&activateEvent);

	event->consume();
}

void GraphControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect rc = getInnerRect();

	// Select font from settings.
	canvas.setFont(m_paintSettings->getFont());

	// Draw background.
	if (m_imageBackground)
	{
		Size backgroundSize = m_imageBackground->getSize();

		int32_t ox = m_offset.cx % backgroundSize.cx;
		int32_t oy = m_offset.cy % backgroundSize.cy;

		canvas.setBackground(Color4ub(255, 255, 255, 255));
		for (int32_t y = oy - backgroundSize.cy; y < rc.getHeight(); y += backgroundSize.cy)
		{
			for (int32_t x = ox - backgroundSize.cx; x < rc.getWidth(); x += backgroundSize.cx)
			{
				canvas.drawBitmap(
					Point(x, y),
					Point(0, 0),
					backgroundSize,
					m_imageBackground,
					ui::BmNone
				);
			}
		}
	}
	else
	{
		canvas.setBackground(getSystemColor(ScButtonFace));
		canvas.fillRect(rc);
	}

	// Draw arrow hints.
	canvas.setBackground(m_paintSettings->getNodeShadow());
	Point center = rc.getCenter();
	unsigned arrowsDrawn = 0;
	for (RefArray< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
	{
		Rect rcNode = (*i)->calculateRect().offset(m_offset);
		if ((arrowsDrawn & 1) == 0 && rcNode.left < rc.left)
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
		if ((arrowsDrawn & 2) == 0 && rcNode.top < rc.top)
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
		if ((arrowsDrawn & 4) == 0 && rcNode.right > rc.right)
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
		if ((arrowsDrawn & 8) == 0 && rcNode.bottom > rc.bottom)
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

	// Draw edges.
	for (RefArray< Edge >::iterator i = m_edges.begin(); i != m_edges.end(); ++i)
	{
		if (!(*i)->isSelected())
			(*i)->paint(m_paintSettings, &canvas, m_offset);
	}

	// Node shapes.
	for (RefArray< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
	{
		if ((*i)->calculateRect().offset(m_offset).intersect(rc))
			(*i)->paint(m_paintSettings, &canvas, m_offset);
	}

	// Draw selected edges.
	for (RefArray< Edge >::iterator i = m_edges.begin(); i != m_edges.end(); ++i)
	{
		if ((*i)->isSelected())
			(*i)->paint(m_paintSettings, &canvas, m_offset);
	}

	// Draw probe.
	if (!m_probeText.empty())
	{
		canvas.setForeground(Color4ub(64, 255, 64, 200));
		canvas.setFont(m_paintSettings->getFontProbe());
		canvas.drawText(m_probeAt, m_probeText);

	}

	// Edge cursor.
	if (m_mode == MdConnectEdge || m_mode == MdDrawEdge)
	{
		canvas.setBackground(m_paintSettings->getGridBackground());
		canvas.setForeground(m_paintSettings->getEdgeCursor());
		canvas.drawLine(m_edgeOrigin, m_cursor);
	}

	// Selection rectangle.
	if (m_mode == MdDrawSelectionRectangle)
	{
		canvas.setForeground(Color4ub(220, 220, 255, 200));
		canvas.setBackground(Color4ub(90, 90, 120, 80));
		canvas.fillRect(Rect(m_moveOrigin, m_cursor));
		canvas.drawRect(Rect(m_moveOrigin, m_cursor));
	}

	event->consume();
}

		}
	}
}
