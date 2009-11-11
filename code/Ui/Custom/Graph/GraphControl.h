#ifndef traktor_ui_custom_GraphControl_H
#define traktor_ui_custom_GraphControl_H

#define T_GRAPH_USE_XTRME 1

#include <list>
#include "Core/Heap/Ref.h"
#include "Core/Heap/RefArray.h"
#if T_GRAPH_USE_XTRME
#	include "Ui/Xtrme/WidgetXtrme.h"
#else
#	include "Ui/Widget.h"
#endif
#include "Ui/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Bitmap;

		namespace custom
		{

class PaintSettings;
class Node;
class Edge;
class Pin;

/*! \brief Graph control.
 * \ingroup UIC
 */
class T_DLLCLASS GraphControl
#if T_GRAPH_USE_XTRME
:	public xtrme::WidgetXtrme
#else
:	public Widget
#endif
{
	T_RTTI_CLASS(GraphControl)

public:
	enum Styles
	{
		WsEdgeSelectable = WsUser
	};

	enum Events
	{
		EiNodeDoubleClick	= EiUser + 1,
		EiNodeMoved			= EiUser + 2,
		EiEdgeConnect		= EiUser + 3,
		EiEdgeDisconnect	= EiUser + 4
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

	virtual void destroy();

	void addNode(Node* node);

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

	/*! \brief Get edges connected to given pin.
	 *
	 * \param pin Pin from which we want to find edges.
	 * \param outEdges Found edges.
	 * \return Number of edges.
	 */
	int getConnectedEdges(const Pin* pin, RefArray< Edge >& outEdges) const;

	/*! \brief Get edges connected to given node.
	 *
	 * \param node Node from which we want to find edges.
	 * \param outEdges Found edges.
	 * \return Number of edges.
	 */
	int getConnectedEdges(const Node* node, RefArray< Edge >& outEdges) const;

	/*! \brief Get edges connected to given nodes.
	 *
	 * \param nodes Nodes from which we want to find edges.
	 * \param inclusive True if both endpoints of an edge must be in nodes array, false if only one must be in the array.
	 * \param outEdges Found edges.
	 * \return Number of edges.
	 */
	int getConnectedEdges(const RefArray< Node >& nodes, bool inclusive, RefArray< Edge >& outEdges) const;

	Ref< Node > getNodeAt(const Point& p) const;

	Ref< Edge > getEdgeAt(const Point& p) const;

	void setPaintSettings(PaintSettings* paintSettings);

	Ref< PaintSettings > getPaintSettings() const;

	void center();

	void alignNodes(Alignment align);

	void evenSpace(EvenSpace space);

	void addSelectEventHandler(EventHandler* eventHandler);

	void addNodeDoubleClickEventHandler(EventHandler* eventHandler);

	void addNodeMovedEventHandler(EventHandler* eventHandler);

	void addEdgeConnectEventHandler(EventHandler* eventHandler);

	void addEdgeDisconnectEventHandler(EventHandler* eventHandler);

	inline const Size& getOffset() const { return m_offset; }

private:
	Ref< PaintSettings > m_paintSettings;
	Ref< Bitmap > m_imageBackground;
	RefArray< Node > m_nodes;
	RefArray< Edge > m_edges;
	Size m_offset;
	Point m_origin;
	Point m_cursor;
	int m_mode;
	bool m_edgeSelectable;
	Ref< Pin > m_selectedPin;
	Ref< Node > m_selectedNode;
	Ref< Edge > m_selectedEdge;
	std::vector< Point > m_nodePositions;
	std::vector< bool > m_nodeSelectionStates;
	std::vector< bool > m_edgeSelectionStates;

	void beginSelectModification();

	void endSelectModification();

	void eventMouseDown(Event* e);

	void eventMouseUp(Event* e);

	void eventMouseMove(Event* e);

	void eventDoubleClick(Event* e);

	void eventPaint(Event* e);
};

		}
	}
}

#endif	// traktor_ui_custom_GraphControl_H
