#ifndef traktor_ui_WidgetGtkImpl_H
#define traktor_ui_WidgetGtkImpl_H

#include <cassert>
#include <vector>
#include <gtk/gtk.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Canvas.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/AllEvents.h"
#include "Ui/Gtk/BareContainer.h"
#include "Ui/Gtk/CanvasGtk.h"
#include "Ui/Gtk/Warp.h"

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
	,	m_parent(nullptr)
	,	m_visible(true)
	,	m_capture(false)
	{
	}

	virtual ~WidgetGtkImpl()
	{
		T_FATAL_ASSERT(m_warp.widget == nullptr);
		T_FATAL_ASSERT(m_timers.empty());
	}

	virtual void destroy()
	{
		// Remove all timers.
		for (auto timer : m_timers)
			g_source_destroy(timer.source);
		m_timers.clear();

		// Remove myself from parent.
		if (m_parent != nullptr)
		{
			Warp* parentWarp = static_cast< Warp* >(m_parent->getInternalHandle());
			if (m_warp.widget != nullptr)
				gtk_container_remove(GTK_CONTAINER(parentWarp->widget), m_warp.widget);
			m_parent = nullptr;
		}

		// Remove widget.
		if (m_warp.widget != nullptr)
		{
			gtk_widget_destroy(m_warp.widget);
			m_warp.widget = nullptr;
		}

		delete this;
	}

	virtual void setParent(IWidget* parent)
	{
		// Remove myself from old parent.
		if (m_parent != nullptr)
		{
			Warp* parentWarp = static_cast< Warp* >(m_parent->getInternalHandle());
			if (m_warp.widget != nullptr)
				gtk_container_remove(GTK_CONTAINER(parentWarp->widget), m_warp.widget);
			m_parent = nullptr;
		}

		// Add myself to new parent.
		if ((m_parent = parent) != nullptr)
		{
			Warp* parentWarp = static_cast< Warp* >(m_parent->getInternalHandle());
			if (m_warp.widget != nullptr)
				gtk_container_add(GTK_CONTAINER(parentWarp->widget), m_warp.widget);
		}
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

	virtual void setForeground()
	{
	}

	virtual bool isForeground() const
	{
	}

	virtual void setVisible(bool visible)
	{
		if (m_warp.widget != nullptr)
			gtk_widget_set_visible(m_warp.widget, visible);
		m_visible = visible;
	}

	virtual bool isVisible(bool includingParents) const
	{
		return m_visible;
	}

	virtual void setActive()
	{
	}

	virtual void setEnable(bool enable)
	{
		if (m_warp.widget != nullptr)
			gtk_widget_set_sensitive(m_warp.widget, enable);
	}

	virtual bool isEnable() const
	{
		if (m_warp.widget != nullptr)
			return gtk_widget_get_sensitive(m_warp.widget);
		else
			return false;
	}

	virtual bool hasFocus() const
	{
		if (m_warp.widget != nullptr)
			return gtk_widget_is_focus(m_warp.widget);
		else
			return false;
	}

	virtual void setFocus()
	{
		if (m_warp.widget != nullptr)
			gtk_widget_grab_focus(m_warp.widget);
	}

	virtual bool hasCapture() const
	{
		return m_capture;
	}

	virtual void setCapture()
	{
		if (!m_capture)
		{
			gtk_grab_add(m_warp.widget);
			m_capture = true;
		}
	}

	virtual void releaseCapture()
	{
		if (m_capture)
		{
			gtk_grab_remove(m_warp.widget);
			m_capture = false;
		}
	}

	virtual void startTimer(int interval, int id)
	{
		Timer t;
		t.id = id;
		t.source = g_timeout_source_new(interval);
		g_source_set_callback(t.source, function_timeout, this, nullptr);
		g_source_set_name(t.source, "Traktor Timer");
		g_source_attach(t.source, nullptr);
		m_timers.push_back(t);
	}

	virtual void stopTimer(int id)
	{
		//g_source_remove(m_timers[id]);
		//m_timers.erase(id);
	}

	virtual void setOutline(const Point* p, int np)
	{
	}

	virtual void setRect(const Rect& rect)
	{
		if (m_parent != nullptr)
		{
			Warp* parentWarp = static_cast< Warp* >(m_parent->getInternalHandle());

			// Place widget.
			if (m_warp.widget != nullptr)
			{
				t_base_container_place_child(
					T_BARE_CONTAINER(parentWarp->widget),
					m_warp.widget,
					rect.left,
					rect.top,
					rect.getWidth(),
					rect.getHeight()
				);
			}
		}
		m_rect = rect;
	}

	virtual Rect getRect() const
	{
		return m_rect;
	}

	virtual Rect getInnerRect() const
	{
		return Rect(0, 0, m_rect.getWidth(), m_rect.getHeight());
	}

	virtual Rect getNormalRect() const
	{
		return m_rect;
	}

	virtual Size getTextExtent(const std::wstring& text) const
	{
		log::info << L"WidgetGtkImpl< " << type_name(m_owner) << L" >::getTextExtent NOT IMPLEMENTED" << Endl;
		return Size(0, 0);
	}

	virtual void setFont(const Font& font)
	{
		log::info << L"WidgetGtkImpl< " << type_name(m_owner) << L" >::setFont NOT IMPLEMENTED" << Endl;
	}

	virtual Font getFont() const
	{
		log::info << L"WidgetGtkImpl< " << type_name(m_owner) << L" >::getFont NOT IMPLEMENTED" << Endl;
		return Font();
	}

	virtual void setCursor(Cursor cursor)
	{
		log::info << L"WidgetGtkImpl< " << type_name(m_owner) << L" >::setCursor NOT IMPLEMENTED" << Endl;
	}

	virtual Point getMousePosition(bool relative) const
	{
		if (relative)
		{
			gint x, y;
			gtk_widget_get_pointer(m_warp.widget, &x, &y);
			return Point(x, y);
		}

		log::info << L"WidgetGtkImpl< " << type_name(m_owner) << L" >::getMousePosition(false) NOT IMPLEMENTED" << Endl;
		return Point(0, 0);
	}

	virtual Point screenToClient(const Point& pt) const
	{
		log::info << L"WidgetGtkImpl< " << type_name(m_owner) << L" >::screenToClient NOT IMPLEMENTED" << Endl;
		return Point(0, 0);
	}

	virtual Point clientToScreen(const Point& pt) const
	{
		log::info << L"WidgetGtkImpl< " << type_name(m_owner) << L" >::clientToScreen NOT IMPLEMENTED" << Endl;
		return Point(0, 0);
	}

	virtual bool hitTest(const Point& pt) const
	{
		log::info << L"WidgetGtkImpl< " << type_name(m_owner) << L" >::hitTest NOT IMPLEMENTED" << Endl;
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
		GtkRequisition mn = { 0 };
		if (m_warp.widget != nullptr)
			gtk_widget_get_preferred_size(m_warp.widget, &mn, nullptr);
		return Size(mn.width, mn.height);
	}

	virtual Size getPreferedSize() const
	{
		GtkRequisition nt = { 0 };
		if (m_warp.widget != nullptr)
			gtk_widget_get_preferred_size(m_warp.widget, nullptr, &nt);
		return Size(nt.width, nt.height);
	}

	virtual Size getMaximumSize() const
	{
		return Size(65535, 65535);
	}

	virtual void update(const Rect* rc, bool immediate)
	{
		if (m_warp.widget != nullptr)
		{
			T_FATAL_ASSERT(GTK_IS_WIDGET(m_warp.widget));
			gtk_widget_queue_draw(m_warp.widget);
		}
	}

	virtual void* getInternalHandle()
	{
		return &m_warp;
	}

	virtual SystemWindow getSystemWindow()
	{
/*		
		// Get internal GTK widget from wxGTK.
		GtkWidget* internalWidget = m_window->m_wxwindow;
		T_ASSERT (internalWidget);

		// Hack, disable GTK double buffering. This method is currently
		// only called when creating a render widget.
		gtk_widget_set_double_buffered(internalWidget, FALSE);

		if (!GTK_WIDGET_REALIZED(internalWidget))
			gtk_widget_realize(internalWidget);
*/
		return SystemWindow(0, 0); //GDK_WINDOW_XWINDOW(m_api.widget->window));		
	}

protected:
	struct Timer
	{
		int32_t id;
		GSource* source;
	};

	EventSubject* m_owner;
	IWidget* m_parent;
	Warp m_warp;
	Rect m_rect;
	bool m_visible;
	bool m_capture;
	std::wstring m_text;
	std::vector< Timer > m_timers;

	bool create(IWidget* parent)
	{
		// All widgets must create a widget.
		if (m_warp.widget == nullptr)
			return false;

		// Connect signals to widget.
		gtk_widget_set_events(
			m_warp.widget,
			GDK_POINTER_MOTION_MASK |
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK
		);

		g_signal_connect(m_warp.widget, "size-allocate", G_CALLBACK(WidgetGtkImpl::signal_size_allocate), this);
		g_signal_connect(m_warp.widget, "motion-notify-event", G_CALLBACK(WidgetGtkImpl::signal_motion_notify), this);
		g_signal_connect(m_warp.widget, "button-press-event", G_CALLBACK(WidgetGtkImpl::signal_button_press), this);
		g_signal_connect(m_warp.widget, "button-release-event", G_CALLBACK(WidgetGtkImpl::signal_button_release), this);
		g_signal_connect(m_warp.widget, "focus-in-event", G_CALLBACK(WidgetGtkImpl::signal_focus_in), this);
		g_signal_connect(m_warp.widget, "focus-out-event", G_CALLBACK(WidgetGtkImpl::signal_focus_out), this);

		gtk_widget_set_name(m_warp.widget, wstombs(type_name(m_owner)).c_str());
		gtk_widget_show(m_warp.widget);

		m_parent = parent;
		return true;
	}

	static gboolean function_timeout(gpointer data)
	{
		WidgetGtkImpl* self = static_cast< WidgetGtkImpl* >(data);
		T_FATAL_ASSERT(self != nullptr);

		TimerEvent timerEvent(self->m_owner, 0);
		self->m_owner->raiseEvent(&timerEvent);

		return G_SOURCE_CONTINUE;
	}

	static void signal_size_allocate(GtkWidget* widget, GdkRectangle* allocation, gpointer data)
	{
		WidgetGtkImpl* self = static_cast< WidgetGtkImpl* >(data);
		T_FATAL_ASSERT(self != nullptr);

		if (self->m_parent != nullptr)
		{
			Warp* parentWarp = static_cast< Warp* >(self->m_parent->getInternalHandle());

			int32_t x = 0, y = 0;
			t_base_container_get_child_position(T_BARE_CONTAINER(parentWarp->widget), GTK_WIDGET(self->m_warp.widget), &x, &y);

			self->m_rect = Rect(
				Point(x, y),
				Size(allocation->width, allocation->height)
			);
		}
		else
		{
			self->m_rect = Rect(
				Point(0, 0),
				Size(allocation->width, allocation->height)
			);
		}

		SizeEvent sizeEvent(self->m_owner, self->m_rect.getSize());
		self->m_owner->raiseEvent(&sizeEvent);
	}

	static gboolean signal_motion_notify(GtkWidget* widget, GdkEventMotion* event, gpointer data)
	{
		WidgetGtkImpl* self = static_cast< WidgetGtkImpl* >(data);
		T_FATAL_ASSERT(self != nullptr);

		int32_t button = MbtNone;
		if (event->state & GDK_BUTTON1_MASK)
			button |= MbtLeft;
		if (event->state & GDK_BUTTON2_MASK)
			button |= MbtMiddle;
		if (event->state & GDK_BUTTON3_MASK)
			button |= MbtRight;

		MouseMoveEvent mouseMoveEvent(
			self->m_owner,
			button,
			Point(int32_t(event->x), int32_t(event->y))
		);
		self->m_owner->raiseEvent(&mouseMoveEvent);
		if (mouseMoveEvent.consumed())
			return TRUE;

		return FALSE;
	}

	static gboolean signal_button_press(GtkWidget* widget, GdkEventButton* event, gpointer data)
	{
		WidgetGtkImpl* self = static_cast< WidgetGtkImpl* >(data);
		T_FATAL_ASSERT(self != nullptr);

		int32_t button = MbtNone;
		if (event->button == 1)
			button = MbtLeft;
		else if (event->button == 2)
			button = MbtMiddle;
		else if (event->button == 3)
			button = MbtRight;

		if (event->type == GDK_BUTTON_PRESS)
		{
			MouseButtonDownEvent mouseDownEvent(
				self->m_owner,
				button,
				Point(int32_t(event->x), int32_t(event->y))
			);
			self->m_owner->raiseEvent(&mouseDownEvent);
			if (mouseDownEvent.consumed())
				return TRUE;
		}
		else if (event->type == GDK_2BUTTON_PRESS)
		{
			MouseDoubleClickEvent mouseDoubleClickEvent(
				self->m_owner,
				button,
				Point(int32_t(event->x), int32_t(event->y))
			);
			self->m_owner->raiseEvent(&mouseDoubleClickEvent);
			if (mouseDoubleClickEvent.consumed())
				return TRUE;
		}

		self->setFocus();

		return FALSE;
	}

	static gboolean signal_button_release(GtkWidget* widget, GdkEventButton* event, gpointer data)
	{
		WidgetGtkImpl* self = static_cast< WidgetGtkImpl* >(data);
		T_FATAL_ASSERT(self != nullptr);

		int32_t button = MbtNone;
		if (event->button == 1)
			button = MbtLeft;
		else if (event->button == 2)
			button = MbtMiddle;
		else if (event->button == 3)
			button = MbtRight;

		MouseButtonUpEvent mouseUpEvent(
			self->m_owner,
			button,
			Point(int32_t(event->x), int32_t(event->y))
		);
		self->m_owner->raiseEvent(&mouseUpEvent);
		if (mouseUpEvent.consumed())
			return TRUE;

		return FALSE;
	}

	static bool signal_focus_in(GtkWidget* widget, GdkEventFocus* event, gpointer data)
	{
		WidgetGtkImpl* self = static_cast< WidgetGtkImpl* >(data);
		T_FATAL_ASSERT(self != nullptr);

		FocusEvent e(
			self->m_owner,
			true
		);
		self->m_owner->raiseEvent(&e);

		return false;
	}

	static bool signal_focus_out(GtkWidget* widget, GdkEventFocus* event, gpointer data)
	{
		WidgetGtkImpl* self = static_cast< WidgetGtkImpl* >(data);
		T_FATAL_ASSERT(self != nullptr);

		FocusEvent e(
			self->m_owner,
			false
		);
		self->m_owner->raiseEvent(&e);

		return false;
	}
};

	}
}

#endif	// traktor_ui_WidgetGtkImpl_H

