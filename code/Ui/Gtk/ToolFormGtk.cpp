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
	gtk_window_set_title(GTK_WINDOW(m_window), wstombs(text).c_str());
	gtk_window_set_default_size(GTK_WINDOW(m_window), width, height);
	//gtk_window_set_border_width(0);

	m_warp.widget = t_bare_container_new();
	gtk_container_add(GTK_CONTAINER(m_window), GTK_WIDGET(m_warp.widget));

	//g_signal_connect(m_window, "delete-event", G_CALLBACK(FormGtk::signal_remove), this);

	m_rect = Rect(0, 0, width, height);

	if (!WidgetGtkImpl< IToolForm >::create(nullptr))
		return false;

	return true;
}

void ToolFormGtk::center()
{
}

void ToolFormGtk::setVisible(bool visible)
{
	gtk_widget_set_visible(m_window, visible);
	WidgetGtkImpl< IToolForm >::setVisible(visible);
}

	}
}

