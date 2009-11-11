#include <algorithm>
#include <limits>
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/PaintSettings.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/Edge.h"
#include "Ui/Custom/Graph/Pin.h"
#include "Ui/Custom/Graph/SelectEvent.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Drawing/Image.h"

// Resources
#include "Resources/Background.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

enum Modes
{
	MdNothing,
	MdMoveAllNodes,
	MdMoveSelectedNodes,
	MdDrawEdge,
	MdDrawSelectionRectangle
};

#if T_GRAPH_USE_XTRME
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GraphControl", GraphControl, xtrme::WidgetXtrme)
#else
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GraphControl", GraphControl, Widget)
#endif

GraphControl::GraphControl()
:	m_mode(MdNothing)
,	m_edgeSelectable(false)
{
}

bool GraphControl::create(Widget* parent, int style)
{
#if T_GRAPH_USE_XTRME
	if (!WidgetXtrme::create(parent, style))
		return false;
#else
	if (!Widget::create(parent, style))
		return false;
#endif

	m_paintSettings = gc_new< PaintSettings >(getFont());
	m_imageBackground = Bitmap::load(c_ResourceBackground, sizeof(c_ResourceBackground), L"png");

	addButtonDownEventHandler(createMethodHandler(this, &GraphControl::eventMouseDown));
	addButtonUpEventHandler(createMethodHandler(this, &GraphControl::eventMouseUp));
	addMouseMoveEventHandler(createMethodHandler(this, &GraphControl::eventMouseMove));
	addDoubleClickEventHandler(createMethodHandler(this, &GraphControl::eventDoubleClick));
#if T_GRAPH_USE_XTRME
	addPaintXtrmeEventHandler(createMethodHandler(this, &GraphControl::eventPaint));
#else
	addPaintEventHandler(createMethodHandler(this, &GraphControl::eventPaint));
#endif

	m_offset.cx =
	m_offset.cy = 0;
	m_origin.x =
	m_origin.y = 0;
	m_cursor.x =
	m_cursor.y = 0;
	m_mode = MdNothing;
	m_edgeSelectable = bool((style & WsEdgeSelectable) == WsEdgeSelectable);

	return true;
}

void GraphControl::destroy()
{
#if T_GRAPH_USE_XTRME
	WidgetXtrme::destroy();
#else
	Widget::destroy();
#endif
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

Ref< Node > GraphControl::getNodeAt(const Point& p) const
{
	for (RefArray< Node >::const_iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
	{
		if ((*i)->hit(p))
			return (*i);
	}
	return 0;
}

Ref< Edge > GraphControl::getEdgeAt(const Point& p) const
{
	for (RefArray< Edge >::const_iterator i = m_edges.begin(); i != m_edges.end(); ++i)
	{
		if ((*i)->hit(m_paintSettings, p))
			return (*i);
	}
	return 0;
}

void GraphControl::setPaintSettings(PaintSettings* paintSettings)
{
	m_paintSettings = paintSettings;
}

Ref< PaintSettings > GraphControl::getPaintSettings() const
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

			Event event(this, *i);
			raiseEvent(GraphControl::EiNodeMoved, &event);
		}
	}
}

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

void GraphControl::evenSpace(EvenSpace space)
{
	RefArray< Node > nodes;
	getSelectedNodes(nodes);

	if (nodes.size() <= 1)
		return;

	std::sort(nodes.begin(), nodes.end(), SortNodePred(space));

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

			Event event(this, *i);
			raiseEvent(GraphControl::EiNodeMoved, &event);
		}

		x += rc.getWidth() + spaceHoriz;
		y += rc.getHeight() + spaceVert;
	}
}

void GraphControl::addSelectEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiSelectionChange, eventHandler);
}

void GraphControl::addNodeDoubleClickEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiNodeDoubleClick, eventHandler);
}

void GraphControl::addNodeMovedEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiNodeMoved, eventHandler);
}

void GraphControl::addEdgeConnectEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiEdgeConnect, eventHandler);
}

void GraphControl::addEdgeDisconnectEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiEdgeDisconnect, eventHandler);
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

void GraphControl::endSelectModification()
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

	if (!nodeSelectChanged.empty() || !edgeSelectChanged.empty())
	{
		SelectEvent selectEvent(this, nodeSelectChanged, edgeSelectChanged);
		raiseEvent(EiSelectionChange, &selectEvent);
	}
}

void GraphControl::eventMouseDown(Event* e)
{
	MouseEvent* m = static_cast< MouseEvent* >(e);

	setFocus();

	m_mode = MdNothing;
	m_selectedPin = 0;
	m_selectedEdge = 0;
	m_selectedNode = 0;

	// Save positions of all nodes so we can issue node moved events later..
	m_nodePositions.resize(m_nodes.size());
	for (uint32_t i = 0; i < m_nodes.size(); ++i)
		m_nodePositions[i] = m_nodes[i]->getPosition();

	// Save origin of drag and where the cursor is currently at.
	m_cursor =
	m_origin = m->getPosition();

	// If user holds down CTRL key and pressed left mouse button we should move entire graph.
	if (e->getKeyState() & KsControl)
	{
		m_mode = MdMoveAllNodes;
		setCapture();
		update();
		e->consume();
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
			if (!(e->getKeyState() & KsShift))
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
		if (m->getButton() == MouseEvent::BtLeft)
		{
			Ref< Pin > pin = m_selectedNode->getPinAt(m_cursor - m_offset);
			if (pin)
			{
				if (pin->getDirection() == Pin::DrOutput)
					m_selectedPin = pin;
				else
				{
					// See if we can find an existing edge connected to this input.
					for (RefArray< Edge >::iterator i = m_edges.begin(); i != m_edges.end(); ++i)
					{
						Ref< Edge > edge = *i;
						if (edge->getDestinationPin() != pin)
							continue;

						m_selectedPin = edge->getSourcePin();
						m_edges.erase(i);

						Event event(this, edge);
						raiseEvent(EiEdgeDisconnect, &event);
						break;
					}
				}

				if (m_selectedPin)
				{
					// Adjust to the center of the pin.
					m_cursor = m_origin = m_selectedPin->getPosition() + m_offset;
					m_mode = MdDrawEdge;
					setCapture();
					update();
					e->consume();
					return;
				}
			}

			// No pin selected, move the selected node(s).
			m_mode = MdMoveSelectedNodes;
			setCapture();
			update();
			e->consume();
		}
	}
	else if (!m_selectedNode && m_selectedEdge)
	{
		T_ASSERT (m_edgeSelectable);
		beginSelectModification();

		// Update selection.
		if (!m_selectedEdge->isSelected())
		{
			if (!(e->getKeyState() & KsShift))
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

		endSelectModification();

		update();
		e->consume();
	}
	else
	{
		// Selection or move must be made by left mouse button.
		if (m->getButton() != MouseEvent::BtLeft)
			return;

		if (!(e->getKeyState() & KsShift))
		{
			beginSelectModification();

			// Deselect all nodes and start drawing selection marker.
			for (RefArray< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
				(*i)->setSelected(false);

			endSelectModification();
		}

		m_mode = MdDrawSelectionRectangle;
		setCapture();
		update();
		e->consume();
	}
}

void GraphControl::eventMouseUp(Event* e)
{
	MouseEvent* m = static_cast< MouseEvent* >(e);
	
	m_cursor = m->getPosition();

	// Connect edge if we were drawing an edge.
	if (m_mode == MdDrawEdge)
	{
		Ref< Node > targetNode = getNodeAt(m_cursor - m_offset);
		if (targetNode)
		{
			Ref< Pin > targetPin = targetNode->getPinAt(m_cursor - m_offset);
			if (targetPin && targetPin->getDirection() == Pin::DrInput)
			{
				Ref< Edge > edge = gc_new< Edge >(
					m_selectedPin,
					targetPin
				);
				
				Event event(this, edge);
				raiseEvent(EiEdgeConnect, &event);
			}
		}
	}

	// Select nodes which are within selection rectangle.
	if (m_mode == MdDrawSelectionRectangle)
	{
		Point tl = m_origin;
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

	// Issue moved event on all moved nodes.
	if (m_mode == MdMoveAllNodes || m_mode == MdMoveSelectedNodes)
	{
		T_ASSERT (m_nodes.size() == m_nodePositions.size());
		for (uint32_t i = 0; i < m_nodes.size(); ++i)
		{
			if (m_nodes[i]->getPosition() != m_nodePositions[i])
			{
				Event event(this, m_nodes[i]);
				raiseEvent(GraphControl::EiNodeMoved, &event);
			}
		}
	}

	m_mode = MdNothing;
	releaseCapture();
	update();

	e->consume();
}

void GraphControl::eventMouseMove(Event* e)
{
	MouseEvent* m = static_cast< MouseEvent* >(e);

	if (m_mode == MdMoveAllNodes)
	{
		m_offset += m->getPosition() - m_origin;
		m_origin = m->getPosition();
		update();
	}
	else if (m_mode == MdMoveSelectedNodes)
	{
		Size offset = m->getPosition() - m_origin;
		for (RefArray< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
		{
			if (m_mode == MdMoveSelectedNodes && !(*i)->isSelected())
				continue;

			Point position = (*i)->getPosition();
			(*i)->setPosition(position + offset);
		}

		m_origin = m->getPosition();
		update();
	}
	else if (m_mode == MdDrawEdge || m_mode == MdDrawSelectionRectangle)
	{
		m_cursor = m->getPosition();
		update();
	}

	e->consume();
}

void GraphControl::eventDoubleClick(Event* e)
{
	MouseEvent* m = static_cast< MouseEvent* >(e);

	if (m->getButton() != MouseEvent::BtLeft || !m_selectedNode)
		return;

	MouseEvent mouseEvent(this, m_selectedNode, m->getButton(), m->getPosition());
	raiseEvent(EiNodeDoubleClick, &mouseEvent);

	e->consume();
}

void GraphControl::eventPaint(Event* e)
{
	PaintEvent* p = static_cast< PaintEvent* >(e);
	Canvas& canvas = p->getCanvas();
	Rect rc = getInnerRect();

	// Select font from settings.
	canvas.setFont(m_paintSettings->getFont());

	// Fill solid background.
	canvas.setBackground(getSystemColor(ScButtonFace));
	canvas.fillRect(rc);

#if T_GRAPH_USE_XTRME

	// Draw background.
	if (m_imageBackground)
	{
		Size backgroundSize = m_imageBackground->getSize();
		canvas.setBackground(Color(255, 255, 255));
		canvas.drawBitmap(
			Point(
				(rc.getWidth() - backgroundSize.cx) / 2,
				(rc.getHeight() - backgroundSize.cy) / 2
			),
			Point(0, 0),
			backgroundSize,
			m_imageBackground,
			ui::BmAlpha
		);
	}

#endif

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
		(*i)->paint(m_paintSettings, &canvas, m_offset);

	// Node shapes.
	for (RefArray< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
	{
		if ((*i)->calculateRect().offset(m_offset).intersect(rc))
			(*i)->paint(m_paintSettings, &canvas, m_offset);
	}

	// Edge cursor.
	if (m_mode == MdDrawEdge)
	{
		canvas.setBackground(m_paintSettings->getGridBackground());
		canvas.setForeground(m_paintSettings->getEdgeCursor());
		canvas.drawLine(m_origin, m_cursor);
	}

	// Selection rectangle.
	if (m_mode == MdDrawSelectionRectangle)
	{
#if T_GRAPH_USE_XTRME

		canvas.setForeground(Color(220, 220, 255, 200));
		canvas.setBackground(Color(90, 90, 120, 80));
		canvas.fillRect(Rect(m_origin, m_cursor));
		canvas.drawRect(Rect(m_origin, m_cursor));

#else	// !T_GRAPH_USE_XTRME

		canvas.setForeground(Color(220, 220, 255));
		canvas.drawRect(Rect(m_origin, m_cursor));

#endif
	}

	e->consume();
}

		}
	}
}
