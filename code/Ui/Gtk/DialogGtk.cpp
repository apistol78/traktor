#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Gtk/DialogGtk.h"

namespace traktor
{
	namespace ui
	{

DialogGtk::DialogGtk(EventSubject* owner)
:	WidgetGtkImpl< IDialog >(owner)
,	m_window(nullptr)
,	m_result(DrCancel)
,	m_modal(false)
{
}

bool DialogGtk::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(m_window), wstombs(text).c_str());
	gtk_window_set_default_size(GTK_WINDOW(m_window), width, height);

	m_warp.widget = t_bare_container_new();
	gtk_container_add(GTK_CONTAINER(m_window), GTK_WIDGET(m_warp.widget));

	g_signal_connect(m_window, "delete-event", G_CALLBACK(DialogGtk::signal_remove), this);

	m_rect = Rect(0, 0, width, height);

	if (!WidgetGtkImpl< IDialog >::create(nullptr))
		return false;

	return true;
}

void DialogGtk::destroy()
{
	WidgetGtkImpl< IDialog >::destroy();
	if (m_window != nullptr)
	{
		gtk_widget_destroy(m_window);
		m_window = nullptr;
	}
}

void DialogGtk::setIcon(ISystemBitmap* icon)
{
}

int DialogGtk::showModal()
{
	m_modal = true;
	gtk_window_set_modal(GTK_WINDOW(m_window), true);
	//gtk_window_set_transient_for();
	gtk_widget_show_all(GTK_WIDGET(m_window));
	gtk_main();
	return m_result;
}

void DialogGtk::endModal(int result)
{
	m_result = result;
	gtk_main_quit();
}

void DialogGtk::setMinSize(const Size& minSize)
{
	log::error << L"DialogGtk::setMinSize NOT implemented!" << Endl;
}

void DialogGtk::setVisible(bool visible)
{
	gtk_widget_set_visible(m_window, visible);
	WidgetGtkImpl< IDialog >::setVisible(visible);
}

gboolean DialogGtk::signal_remove(GtkWidget* widget, GdkEvent* event, gpointer data)
{
	DialogGtk* self = static_cast< DialogGtk* >(data);
	T_FATAL_ASSERT(self != nullptr);

	if (!self->m_modal)
		return FALSE;

	self->m_result = DrCancel;
	gtk_main_quit();
	return TRUE;
}

	}
}
