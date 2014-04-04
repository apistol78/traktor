#ifndef traktor_ui_Widget_H
#define traktor_ui_Widget_H

#include <vector>
#include "Ui/Associative.h"
#include "Ui/Canvas.h"
#include "Ui/Enums.h"
#include "Ui/EventHandler.h"
#include "Ui/EventSubject.h"
#include "Ui/Font.h"
#include "Ui/Rect.h"

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
	
	Ref< Widget > getPreviousSibling() const;
	
	Ref< Widget > getNextSibling() const;
	
	Ref< Widget > getFirstChild() const;
	
	Ref< Widget > getLastChild() const;

	/*! \brief Link/unlink child event.
	 *
	 * Invoked with a ChildEvent when
	 * a child is either linked or unlinked.
	 */
	void addChildEventHandler(EventHandler* eventHandler);

	/*! \brief Key event.
	 *
	 * Invoked with a KeyEvent.
	 */
	void addKeyEventHandler(EventHandler* eventHandler);

	/*! \brief Key down event.
	 *
	 * Invoked with a KeyEvent.
	 */
	void addKeyDownEventHandler(EventHandler* eventHandler);

	/*! \brief Key up event.
	 *
	 * Invoked with a KeyEvent.
	 */
	void addKeyUpEventHandler(EventHandler* eventHandler);

	/*! \brief Move event.
	 *
	 * Issued after the widget has been moved.
	 * Invoked with a MoveEvent.
	 */
	void addMoveEventHandler(EventHandler* eventHandler);
	
	/*! \brief Size event.
	 *
	 * Issued after the widget has been resized.
	 * Invoked with a SizeEvent.
	 */
	void addSizeEventHandler(EventHandler* eventHandler);

	/*! \brief Show event.
	 *
	 * Issued when the widget has either been made visible or hidden.
	 * Invoked with a ShowEvent.
	 */
	void addShowEventHandler(EventHandler* eventHandler);
	
	/*! \brief Mouse button down event.
	 *
	 * Issued when mouse button has been pressed inside the widget.
	 * Invoked with a MouseEvent.
	 */
	void addButtonDownEventHandler(EventHandler* eventHandler);
	
	/*! \brief Mouse button up event.
	 *
	 * Issued when mouse button has been released inside the widget.
	 * Invoked with a MouseEvent.
	 */
	void addButtonUpEventHandler(EventHandler* eventHandler);

	/*! \brief Mouse button double click event.
	 *
	 * Issued when mouse button has been double clicked inside the widget.
	 * Invoked with a MouseEvent.
	 */
	void addDoubleClickEventHandler(EventHandler* eventHandler);
	
	/*! \brief Mouse move event.
	 *
	 * Issued when mouse cursor has been moved inside the widget.
	 * Invoked with a MouseEvent.
	 */
	void addMouseMoveEventHandler(EventHandler* eventHandler);

	/*! \brief Mouse wheel event.
	 *
	 * Issued when mouse wheel has been turned inside the widget.
	 * Invoked with a MouseEvent.
	 */
	void addMouseWheelEventHandler(EventHandler* eventHandler);

	/*! \brief Input focus event.
	 *
	 * Issued when the widget has either gained to lost input focus.
	 * Invoked with a FocusEvent.
	 */
	void addFocusEventHandler(EventHandler* eventHandler);
	
	/*! \brief Paint event.
	 *
	 * Issued when the widget needs repainting it's client area.
	 * Invoked with a PaintEvent.
	 */
	void addPaintEventHandler(EventHandler* eventHandler);
	
	/*! \brief Timer event.
	 *
	 * Issued periodically from a user defined timer.
	 * Invoked with a CommandEvent.
	 */
	void addTimerEventHandler(EventHandler* eventHandler);

	/*! \brief File drop event.
	 *
	 * Issued when user "drops" a file into this widget.
	 * Invoked with a FileDropEvent.
	 *
	 * \note
	 * Widget must be created with WsAcceptFileDrop style.
	 */
	void addFileDropEventHandler(EventHandler* eventHandler);

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
