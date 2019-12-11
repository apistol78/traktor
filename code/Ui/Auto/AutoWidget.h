#pragma once

#include <list>
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

class AutoWidgetCell;
class ScrollBar;

/*! Auto layout widget.
 * \ingroup UI
 *
 * AutoWidget simplifies creation of custom widgets
 * by managing pieces of the widget through AutoWidgetCell classes.
 * The class automates hot-item tracking, mouse capture and
 * animation updates.
 */
class T_DLLCLASS AutoWidget : public Widget
{
	T_RTTI_CLASS;

public:
	AutoWidget();

	bool create(ui::Widget* parent, int32_t style);

	void setFocusCell(AutoWidgetCell* focusCell);

	AutoWidgetCell* getFocusCell() const;

	AutoWidgetCell* hitTest(const Point& position);

	void requestUpdate();

	void requestInterval(AutoWidgetCell* cell, int32_t duration);

	void placeCell(AutoWidgetCell* cell, const Rect& rc);

	void placeHeaderCell(AutoWidgetCell* cell, int32_t height);

	void placeFooterCell(AutoWidgetCell* cell, int32_t height);

	Rect getCellRect(const AutoWidgetCell* cell) const;

	Rect getCellClientRect(const AutoWidgetCell* cell) const;

	bool setCapturedCell(AutoWidgetCell* cell);

	void releaseCapturedCell();

	const Size& getScrollOffset() const;

	Point getClientPosition(const Point& innerPosition) const;

	virtual void layoutCells(const Rect& rc) = 0;

private:
	struct CellInstance
	{
		Ref< AutoWidgetCell > cell;
		Rect rc;
	};

	struct CellInterval
	{
		Ref< AutoWidgetCell > cell;
		int32_t duration;
	};

	std::vector< CellInstance > m_cells;
	CellInstance m_headerCell;
	CellInstance m_footerCell;
	std::list< CellInterval > m_intervals;
	Ref< AutoWidgetCell > m_focusCell;
	Ref< AutoWidgetCell > m_captureCell;
	Ref< ScrollBar > m_scrollBarH;
	Ref< ScrollBar > m_scrollBarV;
	Size m_scrollOffset;
	Rect m_bounds;
	bool m_deferredUpdate;

	void updateLayout();

	void placeScrollBars();

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventDoubleClick(MouseDoubleClickEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);

	void eventPaint(PaintEvent* event);

	void eventSize(SizeEvent* event);

	void eventTimer(TimerEvent* event);

	void eventScroll(ScrollEvent* event);
};

	}
}

