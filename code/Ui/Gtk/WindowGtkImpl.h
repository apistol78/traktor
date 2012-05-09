#ifndef traktor_ui_WindowGtkImpl_H
#define traktor_ui_WindowGtkImpl_H

#include <cassert>
#include <gtkmm.h>
#include "Core/Misc/TString.h"
#include "Ui/EventSubject.h"
#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

template < typename ControlType >
class WindowGtkImpl : public ControlType
{
public:
	WindowGtkImpl(EventSubject* owner)
	:	m_owner(owner)
	,	m_window(0)
	,	m_container(0)
	{
	}

	virtual ~WindowGtkImpl()
	{
		T_ASSERT (m_window == 0);
		T_ASSERT (m_container == 0);
	}

	virtual void destroy()
	{
		delete m_container; m_container = 0;
		delete m_window; m_window = 0;
		delete this;
	}

	virtual void setStyle(int style)
	{
	}

	virtual void setParent(IWidget* parent)
	{
	}

	virtual void setText(const std::wstring& text)
	{
		m_window->set_title(wstombs(text).c_str());
	}

	virtual std::wstring getText() const
	{
		return L"";
	}

	virtual void setToolTipText(const std::wstring& text)
	{
	}

	virtual void setForeground()
	{
	}

	virtual bool isForeground() const
	{
		return m_window->get_is_toplevel();
	}

	virtual void setVisible(bool visible)
	{
		m_window->set_visible(visible);
	}

	virtual bool isVisible(bool includingParents) const
	{
		return m_window->get_visible();
	}

	virtual void setActive()
	{
	}

	virtual void setEnable(bool enable)
	{
	}

	virtual bool isEnable() const
	{
		return true;
	}

	virtual bool hasFocus() const
	{
		return m_window->has_focus();
	}

	virtual bool containFocus() const
	{
		return m_window->has_focus();
	}

	virtual void setFocus()
	{
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
		m_window->resize(rect.getWidth(), rect.getHeight());
	}

	virtual Rect getRect() const
	{
		Gtk::Allocation allocation = m_window->get_allocation();
		return Rect(
			allocation.get_x(),
			allocation.get_y(),
			allocation.get_x() + allocation.get_width(),
			allocation.get_y() + allocation.get_height()
		);
	}

	virtual Rect getInnerRect() const
	{
		Gtk::Allocation allocation = m_container->get_allocation();
		return Rect(
			0,
			0,
			allocation.get_width(),
			allocation.get_height()
		);
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
		for (std::vector< IWidgetRect >::const_iterator i = childRects.begin(); i != childRects.end(); ++i)
		{
			if (i->widget)
				i->widget->setRect(i->rect);
		}
	}

	virtual Size getMinimumSize() const
	{
		return Size(0, 0);
	}

	virtual Size getPreferedSize() const
	{
		return Size(0, 0);
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
		return m_container;
	}

	virtual void* getSystemHandle()
	{
		return 0;
	}

protected:
	EventSubject* m_owner;
	Gtk::Window* m_window;
	Gtk::Container* m_container;
};

	}
}

#endif	// traktor_ui_WindowGtkImpl_H

