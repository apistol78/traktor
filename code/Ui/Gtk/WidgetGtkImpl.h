#ifndef traktor_ui_WidgetGtkImpl_H
#define traktor_ui_WidgetGtkImpl_H

#include <cassert>
#include <gtkmm.h>
#include "Ui/Itf/IWidget.h"
#include "Ui/EventSubject.h"

namespace traktor
{
	namespace ui
	{

template < typename ControlType >
class WidgetGtkImpl : public ControlType
{
public:
	WidgetGtkImpl(EventSubject* owner)
	:	m_owner(owner)
	,	m_widget(0)
	{
	}

	virtual ~WidgetGtkImpl()
	{
		T_ASSERT (m_widget == 0);
	}

	virtual void destroy()
	{
		delete m_widget; m_widget = 0;
		delete this;
	}

	virtual void setParent(IWidget* parent)
	{
	}

	virtual void setText(const std::wstring& text)
	{
	}

	virtual std::wstring getText() const
	{
		return L"";
	}

	virtual void setStyle(int style)
	{
	}

	virtual void setToolTipText(const std::wstring& text)
	{
	}

	virtual void setForeground()
	{
	}

	virtual bool isForeground() const
	{
	}

	virtual void setVisible(bool visible)
	{
	}

	virtual bool isVisible(bool includingParents) const
	{
		return true;
	}

	virtual void setActive()
	{
	}

	virtual void setEnable(bool enable)
	{
		m_widget->set_sensitive(enable);
	}

	virtual bool isEnable() const
	{
		return true;
	}

	virtual bool hasFocus() const
	{
		return false;
	}

	virtual bool containFocus() const
	{
		return false;
	}

	virtual void setFocus()
	{
		m_widget->grab_focus();
	}

	virtual bool hasCapture() const
	{
		return false;
	}

	virtual void setCapture()
	{
	}

	virtual void releaseCapture()
	{
	}

	virtual void startTimer(int interval, int id)
	{
	}

	virtual void stopTimer(int id)
	{
	}

	virtual void setOutline(const Point* p, int np)
	{
	}

	virtual void setRect(const Rect& rect)
	{
		m_parentContainer->put(*m_widget, rect.left, rect.top);
	}

	virtual Rect getRect() const
	{
		return Rect(0, 0, 0, 0);
	}

	virtual Rect getInnerRect() const
	{
		return Rect(0, 0, 0, 0);
	}

	virtual Rect getNormalRect() const
	{
		return Rect(0, 0, 0, 0);
	}

	virtual Size getTextExtent(const std::wstring& text) const
	{
		return Size(0, 0);
	}

	virtual void setFont(const Font& font)
	{
	}

	virtual Font getFont() const
	{
		return Font();
	}

	virtual void setCursor(Cursor cursor)
	{
	}

	virtual Point getMousePosition(bool relative) const
	{
		return Point(0, 0);
	}

	virtual Point screenToClient(const Point& pt) const
	{
		return Point(0, 0);
	}

	virtual Point clientToScreen(const Point& pt) const
	{
		return Point(0, 0);
	}

	virtual bool hitTest(const Point& pt) const
	{
		return false;
	}

	virtual void setChildRects(const std::vector< IWidgetRect >& childRects)
	{
	}

	virtual Size getMinimumSize() const
	{
		return Size(0, 0);
	}

	virtual Size getPreferedSize() const
	{
	    Size minimumSize = getMinimumSize();

	    Gtk::Requisition ms;
	    Gtk::Requisition ns;

	    ms.width = minimumSize.cx;
	    ms.height = minimumSize.cy;

	    m_widget->get_preferred_size(ms, ns);

		return Size(ns.width, ns.height);
	}

	virtual Size getMaximumSize() const
	{
		return Size(65535, 65535);
	}

	virtual void update(const Rect* rc, bool immediate)
	{
	}

	virtual void* getInternalHandle()
	{
		return m_widget;
	}

	virtual void* getSystemHandle()
	{
		return 0;
	}

protected:
	EventSubject* m_owner;
	Gtk::Widget* m_widget;
	Gtk::Fixed* m_parentContainer;
};

	}
}

#endif	// traktor_ui_WidgetGtkImpl_H

