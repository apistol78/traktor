#ifndef traktor_ui_WindowGtkImpl_H
#define traktor_ui_WindowGtkImpl_H

#include <cassert>
#include <gtkmm.h>
#include "Ui/Itf/IWidget.h"
#include "Ui/EventSubject.h"

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

	virtual void setText(const std::string& text)
	{
		m_window->set_title(text.c_str());
	}

	virtual std::string getText() const
	{
		return "";
	}

	virtual void setToolTipText(const std::string& text)
	{
	}

	virtual bool isTopLevel() const
	{
		return true;
	}

	virtual void setVisible(bool visible)
	{
	}

	virtual bool isVisible() const
	{
		return true;
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
		return false;
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
		gint x, y, width, height;
		//gtk_window_get_position(GTK_WINDOW(m_widget), &x, &y);
		//gtk_window_get_size(GTK_WINDOW(m_widget), &width, &height);
		x = y = width = height = 0;
		return Rect(x, y, x + width, y + height);
	}

	virtual Rect getInnerRect() const
	{
		gint width, height;
		//gtk_window_get_size(GTK_WINDOW(m_widget), &width, &height);
		width = height = 0;
		return Rect(0, 0, width, height);
	}

	virtual Rect getNormalRect() const
	{
		return Rect(0, 0, 0, 0);
	}

	virtual Size getTextExtent(const std::string& text) const
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

