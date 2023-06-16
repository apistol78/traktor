/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Associative.h"
#include "Ui/EventSubject.h"
#include "Ui/FontMetric.h"
#include "Ui/Rect.h"
#include "Ui/Unit.h"

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

class AutoWidget;
class Canvas;
class MouseButtonDownEvent;
class MouseButtonUpEvent;
class MouseDoubleClickEvent;
class MouseMoveEvent;
class StyleSheet;

/*! Auto widget cell.
 * \ingroup UI
 */
class T_DLLCLASS AutoWidgetCell
:	public ui::EventSubject
,	public Associative
{
	T_RTTI_CLASS;

public:
	virtual void placeCells(AutoWidget* widget, const Rect& rect);

	virtual Rect getRect() const;

	virtual Rect getClientRect() const;

	virtual AutoWidgetCell* hitTest(const Point& position);

	virtual bool beginCapture();

	virtual void endCapture();

	virtual void interval();

	virtual void mouseEnter();

	virtual void mouseLeave();

	virtual void mouseDown(MouseButtonDownEvent* event, const Point& position);

	virtual void mouseUp(MouseButtonUpEvent* event, const Point& position);

	virtual void mouseDoubleClick(MouseDoubleClickEvent* event, const Point& position);

	virtual void mouseMove(MouseMoveEvent* event, const Point& position);

	virtual void mouseMoveFocus(MouseMoveEvent* event, const Point& position);

	virtual void paint(Canvas& canvas, const Rect& rect);

	const StyleSheet* getStyleSheet() const;

	FontMetric getFontMetric() const;

	int32_t pixel(Unit measure) const;

	Unit unit(int32_t measure) const;

protected:
	void setWidget(AutoWidget* widget)
	{
		T_ASSERT(m_widget == nullptr);
		m_widget = widget;
	}

	AutoWidget* getWidget() const
	{
		return m_widget;
	}

	template < typename WidgetType >
	WidgetType* getWidget() const
	{
		return dynamic_type_cast< WidgetType* >(m_widget);
	}

	void requestWidgetUpdate();

	void raiseWidgetEvent(Event* event);

private:
	AutoWidget* m_widget = nullptr;
	Rect m_rect;
};

	}
}

