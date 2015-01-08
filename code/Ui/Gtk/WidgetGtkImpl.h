#ifndef traktor_ui_WidgetGtkImpl_H
#define traktor_ui_WidgetGtkImpl_H

#include <cassert>
#include <gtkmm.h>
#include "Core/Log/Log.h"
#include "Ui/Canvas.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/AllEvents.h"
#include "Ui/Gtk/CanvasGtk.h"

namespace traktor
{
	namespace ui
	{

template < typename ControlType >
class WidgetGtkImpl : public ControlType
{
public:
	struct Internal
	{
		Gtk::Fixed* container;
		Gtk::Widget* widget;

		Internal()
		:	container(0)
		,	widget(0)
		{
		}
	};

	WidgetGtkImpl(EventSubject* owner)
	:	m_owner(owner)
	{
	}

	virtual ~WidgetGtkImpl()
	{
		T_ASSERT (m_internal.widget == 0);
		T_ASSERT (m_internal.container == 0);
	}

	virtual void destroy()
	{
		if (m_internal.widget != m_internal.container)
		{
			delete m_internal.widget;
			delete m_internal.container;
		}
		else
		{
			delete m_internal.widget;
		}

		m_internal.widget = 0;
		m_internal.container = 0;

		delete this;
	}

	virtual void setParent(IWidget* parent)
	{
		Internal* parentInternal = static_cast< Internal* >(parent->getInternalHandle());
		T_FATAL_ASSERT(parentInternal);

		m_internal.container->reparent(*parentInternal->container);
	}

	virtual void setText(const std::wstring& text)
	{
		m_text = text;
	}

	virtual std::wstring getText() const
	{
		return m_text;
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
		m_internal.container->set_visible(visible);
	}

	virtual bool isVisible(bool includingParents) const
	{
		return m_internal.container->is_visible();
	}

	virtual void setActive()
	{
	}

	virtual void setEnable(bool enable)
	{
		m_internal.widget->set_sensitive(enable);
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
		m_internal.widget->grab_focus();
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
		// Move within parent container.
		Gtk::Fixed* parentContainer = static_cast< Gtk::Fixed* >(m_internal.container->get_parent());
		parentContainer->move(*m_internal.container, rect.left, rect.top);

		Gtk::Allocation allocation(rect.left, rect.top, rect.getWidth(), rect.getHeight());
		m_internal.container->set_allocation(allocation);
		m_internal.widget->set_allocation(allocation);

		m_internal.container->set_size_request(rect.getWidth(), rect.getHeight());
		m_internal.widget->set_size_request(rect.getWidth(), rect.getHeight());
	}

	virtual Rect getRect() const
	{
		Gtk::Allocation allocation = m_internal.container->get_allocation();
		return Rect(
			allocation.get_x(),
			allocation.get_y(),
			allocation.get_x() + allocation.get_width(),
			allocation.get_y() + allocation.get_height()
		);
	}

	virtual Rect getInnerRect() const
	{
		Gtk::Allocation allocation = m_internal.container->get_allocation();
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
		return Size(16, 16);
	}

	virtual Size getPreferedSize() const
	{
	    Size minimumSize = getMinimumSize();

	    Gtk::Requisition ms;
	    Gtk::Requisition ns;

	    ms.width = minimumSize.cx;
	    ms.height = minimumSize.cy;

	    m_internal.widget->get_preferred_size(ms, ns);

		return Size(ns.width, ns.height);
	}

	virtual Size getMaximumSize() const
	{
		return Size(65535, 65535);
	}

	virtual void update(const Rect* rc, bool immediate)
	{
		m_internal.widget->queue_draw();
	}

	virtual void* getInternalHandle()
	{
		return &m_internal;
	}

	virtual void* getSystemHandle()
	{
		return 0;
	}

protected:
	EventSubject* m_owner;
	Internal m_internal;
	std::wstring m_text;

	bool create()
	{
		if (!m_internal.container || !m_internal.widget)
			return false;

		m_internal.container->add_events(Gdk::POINTER_MOTION_MASK);
		m_internal.container->add_events(Gdk::BUTTON_PRESS_MASK);
		m_internal.container->add_events(Gdk::BUTTON_RELEASE_MASK);
		m_internal.container->add_events(Gdk::FOCUS_CHANGE_MASK);

		m_internal.widget->add_events(Gdk::POINTER_MOTION_MASK);
		m_internal.widget->add_events(Gdk::BUTTON_PRESS_MASK);
		m_internal.widget->add_events(Gdk::BUTTON_RELEASE_MASK);
		m_internal.widget->add_events(Gdk::FOCUS_CHANGE_MASK);

		m_internal.widget->signal_size_allocate().connect(sigc::mem_fun(*this, &WidgetGtkImpl< ControlType >::on_size_allocate));
		m_internal.widget->signal_draw().connect(sigc::mem_fun(*this, &WidgetGtkImpl< ControlType >::on_draw));
		m_internal.widget->signal_motion_notify_event().connect(sigc::mem_fun(*this, &WidgetGtkImpl< ControlType >::on_motion_notify));
		m_internal.widget->signal_button_press_event().connect(sigc::mem_fun(*this, &WidgetGtkImpl< ControlType >::on_button_press));
		m_internal.widget->signal_button_release_event().connect(sigc::mem_fun(*this, &WidgetGtkImpl< ControlType >::on_button_release));
		m_internal.widget->signal_focus_in_event().connect(sigc::mem_fun(*this, &WidgetGtkImpl< ControlType >::on_focus_in));
		m_internal.widget->signal_focus_out_event().connect(sigc::mem_fun(*this, &WidgetGtkImpl< ControlType >::on_focus_out));

		m_internal.container->show();
		m_internal.widget->show();

		return true;
	}

	void on_size_allocate(Gtk::Allocation& allocation)
	{
		log::info << L"WidgetGtkImpl::on_size_allocate " << type_name(m_owner) << L" : " << allocation.get_width() << L" x " << allocation.get_height() << Endl;

		SizeEvent sizeEvent(m_owner, Size(allocation.get_width(), allocation.get_height()));
		m_owner->raiseEvent(&sizeEvent);
	}

	bool on_draw(const ::Cairo::RefPtr< ::Cairo::Context >& cr)
	{
		log::info << L"WidgetGtkImpl::on_draw " << type_name(m_owner) << Endl;

		if (!m_owner->hasEventHandler< PaintEvent >())
			return false;

		Rect rc = getInnerRect();

		CanvasGtk canvasGtk(cr);
		Canvas canvas(&canvasGtk);

		PaintEvent paintEvent(m_owner, canvas, rc);
		m_owner->raiseEvent(&paintEvent);

		return paintEvent.consumed();
	}

	bool on_motion_notify(GdkEventMotion* event)
	{
		log::info << L"WidgetGtkImpl::on_motion_notify " << type_name(m_owner) << Endl;

		int32_t button = MbtNone;
		if (event->state & GDK_BUTTON1_MASK)
			button |= MbtLeft;
		if (event->state & GDK_BUTTON2_MASK)
			button |= MbtMiddle;
		if (event->state & GDK_BUTTON3_MASK)
			button |= MbtRight;

		MouseMoveEvent m(
			m_owner,
			button,
			Point(int32_t(event->x), int32_t(event->y))
		);
		m_owner->raiseEvent(&m);

		return false;
	}

	bool on_button_press(GdkEventButton* event)
	{
		log::info << L"WidgetGtkImpl::on_button_press " << type_name(m_owner) << Endl;

		int32_t button = MbtNone;
		if (event->button == 1)
			button = MbtLeft;
		else if (event->button == 2)
			button = MbtMiddle;
		else if (event->button == 3)
			button = MbtRight;

		MouseButtonDownEvent m(
			m_owner,
			button,
			Point(int32_t(event->x), int32_t(event->y))
		);
		m_owner->raiseEvent(&m);

		return false;
	}

	bool on_button_release(GdkEventButton* event)
	{
		log::info << L"WidgetGtkImpl::on_button_release " << type_name(m_owner) << Endl;

		int32_t button = MbtNone;
		if (event->button == 1)
			button = MbtLeft;
		else if (event->button == 2)
			button = MbtMiddle;
		else if (event->button == 3)
			button = MbtRight;

		MouseButtonUpEvent m(
			m_owner,
			button,
			Point(int32_t(event->x), int32_t(event->y))
		);
		m_owner->raiseEvent(&m);

		return false;
	}

	bool on_focus_in(GdkEventFocus* event)
	{
		log::info << L"WidgetGtkImpl::on_focus_in" << Endl;

		FocusEvent e(
			m_owner,
			true
		);
		m_owner->raiseEvent(&e);

		return false;
	}

	bool on_focus_out(GdkEventFocus* event)
	{
		log::info << L"WidgetGtkImpl::on_focus_out" << Endl;

		FocusEvent e(
			m_owner,
			false
		);
		m_owner->raiseEvent(&e);

		return false;
	}
};

	}
}

#endif	// traktor_ui_WidgetGtkImpl_H

