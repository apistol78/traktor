#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Gtk/ToolFormGtk.h"

namespace traktor
{
	namespace ui
	{

ToolFormGtk::ToolFormGtk(EventSubject* owner)
:	WidgetGtkImpl< IToolForm >(owner)
,	m_window(nullptr)
{
}

bool ToolFormGtk::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	if ((style & (WsResizable | WsSystemBox | WsCloseBox | WsCaption)) != 0)
		gtk_window_set_title(GTK_WINDOW(m_window), wstombs(text).c_str());
	else
		gtk_window_set_decorated(GTK_WINDOW(m_window), FALSE);

	gtk_window_set_default_size(GTK_WINDOW(m_window), width, height);
	gtk_window_set_resizable(GTK_WINDOW(m_window), FALSE);

	m_warp.widget = t_bare_container_new();
	gtk_container_add(GTK_CONTAINER(m_window), GTK_WIDGET(m_warp.widget));

	if (parent != nullptr)
	{
		Warp* p = static_cast< Warp* >(parent->getInternalHandle());
		T_FATAL_ASSERT(p != nullptr);

		GtkWindow* parentWindow = GTK_WINDOW(gtk_widget_get_toplevel(p->widget));
		T_FATAL_ASSERT(parentWindow != nullptr);

		gtk_window_set_transient_for(GTK_WINDOW(m_window), parentWindow);
	}

	m_rect = Rect(0, 0, width, height);

	if (!WidgetGtkImpl< IToolForm >::create(parent))
		return false;

	return true;
}

void ToolFormGtk::destroy()
{
	if (m_window != nullptr)
	{
		gtk_widget_destroy(m_window);
		m_window = nullptr;
	}
	WidgetGtkImpl< IToolForm >::destroy();
}

void ToolFormGtk::center()
{
	gtk_window_set_position(GTK_WINDOW(m_window), GTK_WIN_POS_CENTER);
}

void ToolFormGtk::setVisible(bool visible)
{
	gtk_widget_set_visible(m_window, visible);
	WidgetGtkImpl< IToolForm >::setVisible(visible);
}

void ToolFormGtk::setRect(const Rect& rect)
{
	gtk_window_move(GTK_WINDOW(m_window), rect.left, rect.top);
	gtk_window_set_default_size(GTK_WINDOW(m_window), rect.getWidth(), rect.getHeight());
	gtk_window_resize(GTK_WINDOW(m_window), rect.getWidth(), rect.getHeight());
	WidgetGtkImpl< IToolForm >::setRect(rect);
}

	}
}

