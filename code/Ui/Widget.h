/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Widget_H
#define traktor_ui_Widget_H

#include <vector>
#include "Ui/Associative.h"
#include "Ui/Canvas.h"
#include "Ui/Enums.h"
#include "Ui/EventSubject.h"
#include "Ui/Font.h"
#include "Ui/Rect.h"
#include "Ui/Events/AllEvents.h"

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

class IWidget;

/*! \brief Deferred widget rectangle.
 * \ingroup UI
 */
struct WidgetRect
{
	Ref< class Widget > widget;
	Rect rect;

	WidgetRect(class Widget* widget_ = 0, const Rect& rect_ = Rect())
	:	widget(widget_)
	,	rect(rect_)
	{
	}
};

/*! \brief Base widget class.
 * \ingroup UI
 */
class T_DLLCLASS Widget
:	public EventSubject
,	public Associative
{
	T_RTTI_CLASS;

public:
	Widget();
	
	virtual ~Widget();
	
	bool create(Widget* parent, int style = WsNone);
	
	virtual void destroy();
	
	virtual void setText(const std::wstring& text);
	
	virtual std::wstring getText() const;
	
	virtual void setToolTipText(const std::wstring& text);

	virtual void setForeground();

	virtual bool isForeground() const;

	virtual void setVisible(bool visible);

	virtual bool isVisible(bool includingParents) const;
	
	virtual void setEnable(bool enable);
	
	virtual bool isEnable() const;

	virtual bool hasFocus() const;

	virtual bool containFocus() const;

	virtual void setFocus();

	virtual void setRect(const Rect& rect);
	
	virtual Rect getRect() const;
	
	virtual Rect getInnerRect() const;

	virtual Rect getNormalRect() const;

	virtual Size getTextExtent(const std::wstring& text) const;

	virtual void setFont(const Font& font);
	
	virtual Font getFont();
	
	virtual void setCursor(Cursor cursor);
	
	virtual void resetCursor();

	virtual void update(const Rect* rc = 0, bool immediate = false);

	virtual void show();

	virtual void hide();

	void raise();

	void lower();

	bool hasCapture() const;

	void setCapture();
	
	void releaseCapture();
	
	void startTimer(int interval, int id = -1);

	void stopTimer(int id = -1);

	void setOutline(const Point* p, int np);

	Point getMousePosition(bool relative = true) const;

	Point screenToClient(const Point& pt) const;

	Point clientToScreen(const Point& pt) const;

	bool hitTest(const Point& pt) const;

	/*! \brief Update multiple children widgets.
	 *
	 * Use this method when updating multiple children positions or sizes
	 * as it's a lot quicker and results in less flicker than moving one
	 * at a time.
	 */
	void setChildRects(const std::vector< WidgetRect >& childRects);
	
	virtual Size getMinimumSize() const;
	
	virtual Size getPreferedSize() const;
	
	virtual Size getMaximumSize() const;

	/*! \brief If this widget accepts to be part of layout.
	 * For instance child dialogs cannot be part of a layout
	 * as it's not logical, thus should return false.
	 */
	virtual bool acceptLayout() const;
	
	void link(Widget* parent);
	
	void unlink();

	void setParent(Widget* parent);
	
	Widget* getParent() const;
	
	Widget* getPreviousSibling() const;
	
	Widget* getNextSibling() const;
	
	Widget* getFirstChild() const;
	
	Widget* getLastChild() const;

	/*! \brief Get internal widget.
	 *
	 * Retrieve the internal widget object, useful when
	 * getting information about the native peer widget.
	 */
	IWidget* getIWidget() const;
	
protected:
	IWidget* m_widget;
	Widget* m_parent;
	Ref< Widget > m_previousSibling;
	Ref< Widget > m_nextSibling;
	Ref< Widget > m_firstChild;
	Ref< Widget > m_lastChild;
};
	
	}
}

#endif	// traktor_ui_Widget_H
