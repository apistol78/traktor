#ifndef traktor_ui_DockPane_H
#define traktor_ui_DockPane_H

#include "Core/Object.h"
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

class Canvas;

/*! \brief Docking pane.
 * \ingroup UI
 */
class T_DLLCLASS DockPane : public Object
{
	T_RTTI_CLASS;

public:
	enum Direction
	{
		DrNorth,
		DrSouth,
		DrWest,
		DrEast
	};

	DockPane(Widget* owner, DockPane* parent);

	void split(bool vertical, int split, Ref< DockPane >& outLeftPane, Ref< DockPane >& outRightPane);

	void dock(Widget* widget, bool detachable);

	void dock(Widget* widget, bool detachable, Direction direction, int split);

	void undock(Widget* widget);

	void detach();

	Ref< DockPane > findWidgetPane(Widget* widget);

	Ref< DockPane > getPaneFromPosition(const Point& position);

	Ref< DockPane > getSplitterFromPosition(const Point& position);

	bool hitGripper(const Point& position) const;

	bool hitGripperClose(const Point& position) const;

	bool hitSplitter(const Point& position) const;

	void setSplitterPosition(const Point& position);

	bool isSplitter() const { return m_child[0] != 0 && m_child[1] != 0; }

	const Rect& getPaneRect() const { return m_rect; }

	bool isVisible() const;

	void dump();

private:
	friend class Dock;

	Ref< EventSubject::IEventHandler > m_focusEventHandler;
	Widget* m_owner;
	DockPane* m_parent;
	Ref< Widget > m_widget;
	bool m_detachable;
	Ref< DockPane > m_child[2];
	bool m_vertical;
	int m_split;
	Rect m_rect;
	bool m_focus;

	void update(const Rect& rect, std::vector< WidgetRect >& outWidgetRects);

	void draw(Canvas& canvas);

	void eventFocus(FocusEvent* event);
};

	}
}

#endif	// traktor_ui_DockPane_H
