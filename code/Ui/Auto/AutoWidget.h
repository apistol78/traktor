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
	bool create(ui::Widget* parent, uint32_t style);

	void setFocusCell(AutoWidgetCell* focusCell);

	AutoWidgetCell* getFocusCell() const;

	AutoWidgetCell* hitTest(const Point& position);

	AutoWidgetCell* getHoverCell() const;

	AutoWidgetCell* getHeaderCell() const;

	AutoWidgetCell* getFooterCell() const;

	void requestUpdate();

	void requestInterval(AutoWidgetCell* cell, int32_t duration);

	void placeCell(AutoWidgetCell* cell, const Rect& rc);

	void placeHeaderCell(AutoWidgetCell* cell, int32_t height);

	void placeFooterCell(AutoWidgetCell* cell, int32_t height);

	bool setCapturedCell(AutoWidgetCell* cell);

	void releaseCapturedCell();

	void scrollTo(const Point& pnt);

	const Size& getScrollOffset() const;

	Point getClientPosition(const Point& innerPosition) const;

	virtual void layoutCells(const Rect& rc) = 0;

	void updateLayout();

private:
	struct CellInterval
	{
		Ref< AutoWidgetCell > cell;
		int32_t duration;
	};

	RefArray< AutoWidgetCell > m_cells;
	Ref< AutoWidgetCell > m_headerCell;
	Ref< AutoWidgetCell > m_footerCell;
	std::list< CellInterval > m_intervals;
	Ref< AutoWidgetCell > m_focusCell;
	Ref< AutoWidgetCell > m_captureCell;
	Ref< AutoWidgetCell > m_hoverCell;
	Ref< ScrollBar > m_scrollBarH;
	Ref< ScrollBar > m_scrollBarV;
	Size m_scrollOffset = { 0, 0 };
	Rect m_bounds;
	bool m_deferredUpdate = false;

	void placeScrollBars();

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventDoubleClick(MouseDoubleClickEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);

	void eventMouseTrack(MouseTrackEvent* event);

	void eventPaint(PaintEvent* event);

	void eventSize(SizeEvent* event);

	void eventTimer(TimerEvent* event);

	void eventScroll(ScrollEvent* event);
};

	}
}

