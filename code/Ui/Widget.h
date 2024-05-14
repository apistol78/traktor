/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Associative.h"
#include "Ui/Canvas.h"
#include "Ui/Enums.h"
#include "Ui/EventSubject.h"
#include "Ui/Font.h"
#include "Ui/FontMetric.h"
#include "Ui/Rect.h"
#include "Ui/Unit.h"
#include "Ui/Events/AllEvents.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class IWidget;
class StyleSheet;

/*! Deferred widget rectangle.
 * \ingroup UI
 */
struct WidgetRect
{
	class Widget* widget;
	Rect rect;

	WidgetRect(class Widget* widget_ = nullptr, const Rect& rect_ = Rect())
	:	widget(widget_)
	,	rect(rect_)
	{
	}
};

/*! Base widget class.
 * \ingroup UI
 */
class T_DLLCLASS Widget
:	public EventSubject
,	public Associative
{
	T_RTTI_CLASS;

public:
	virtual ~Widget();

	bool create(Widget* parent, uint32_t style = WsNone);

	virtual void destroy();

	virtual void setText(const std::wstring& text);

	virtual std::wstring getText() const;

	virtual void setForeground();

	virtual bool isForeground() const;

	virtual void setVisible(bool visible);

	virtual bool isVisible(bool includingParents) const;

	virtual void setEnable(bool enable);

	virtual bool isEnable(bool includingParents) const;

	virtual bool hasFocus() const;

	virtual bool containFocus() const;

	virtual void setFocus();

	virtual void setRect(const Rect& rect);

	virtual Rect getRect() const;

	virtual Rect getInnerRect() const;

	virtual Rect getNormalRect() const;

	virtual void setFont(const Font& font);

	virtual Font getFont() const;

	virtual FontMetric getFontMetric() const;

	virtual void setCursor(Cursor cursor);

	virtual void resetCursor();

	virtual void update(const Rect* rc = nullptr, bool immediate = false);

	virtual void show();

	virtual void hide();

	int32_t dpi() const;

	int32_t pixel(Unit measure) const;

	Unit unit(int32_t measure) const;

	Point pixel(const UnitPoint& measure) const;

	UnitPoint unit(const Point& pt) const;

	Size pixel(const UnitSize& measure) const;

	UnitSize unit(const Size& sz) const;

	Rect pixel(const UnitRect& measure) const;

	UnitRect unit(const Rect& rc) const;

	bool hasCapture() const;

	void setCapture();

	void releaseCapture();

	void startTimer(int interval);

	void stopTimer();

	Point getMousePosition(bool relative = true) const;

	Point screenToClient(const Point& pt) const;

	Point clientToScreen(const Point& pt) const;

	bool hitTest(const Point& pt) const;

	/*! Update multiple children widgets.
	 *
	 * Use this method when updating multiple children positions or sizes
	 * as it's a lot quicker and results in less flicker than moving one
	 * at a time.
	 */
	void setChildRects(const WidgetRect* childRects, uint32_t count, bool redraw);

	virtual Size getMinimumSize() const;

	virtual Size getPreferredSize(const Size& hint) const;

	virtual Size getMaximumSize() const;

	void setHorizontalAlign(Align halign);

	Align getHorizontalAlign() const;

	void setVerticalAlign(Align valign);

	Align getVerticalAlign() const;

	void setStyleSheet(const StyleSheet* styleSheet);

	const StyleSheet* getStyleSheet() const;

	/*! If this widget accepts to be part of layout.
	 * For instance child dialogs cannot be part of a layout
	 * as it's not logical, thus should return false.
	 */
	virtual bool acceptLayout() const;

	void link(Widget* parent);

	void unlink();

	void setParent(Widget* parent);

	Widget* getParent() const;

	Widget* getAncestor() const;

	Widget* getPreviousSibling() const;

	Widget* getNextSibling() const;

	Widget* getFirstChild() const;

	Widget* getLastChild() const;

	/*! Get internal widget.
	 *
	 * Retrieve the internal widget object, useful when
	 * getting information about the native peer widget.
	 */
	IWidget* getIWidget() const;

protected:
	IWidget* m_widget = nullptr;
	Widget* m_parent = nullptr;
	Ref< Widget > m_previousSibling;
	Ref< Widget > m_nextSibling;
	Ref< Widget > m_firstChild;
	Ref< Widget > m_lastChild;
	Ref< const StyleSheet > m_styleSheet;
	Align m_halign = AnLeft;
	Align m_valign = AnTop;
};

}
