#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Gtk/BareContainer.h"
#include "Ui/Gtk/FormGtk.h"

namespace traktor
{
	namespace ui
	{

FormGtk::FormGtk(EventSubject* owner)
:	WidgetGtkImpl< IForm >(owner)
,	m_window(nullptr)
{
}

bool FormGtk::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	T_FATAL_ASSERT(parent == nullptr);

	m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(m_window), wstombs(text).c_str());
	gtk_window_set_default_size(GTK_WINDOW(m_window), width, height);

	m_warp.widget = t_bare_container_new();
	gtk_container_add(GTK_CONTAINER(m_window), GTK_WIDGET(m_warp.widget));

	g_signal_connect(m_window, "delete-event", G_CALLBACK(FormGtk::signal_remove), this);

	m_rect = Rect(0, 0, width, height);

	if (!WidgetGtkImpl< IForm >::create(nullptr))
		return false;

	return true;
}

void FormGtk::setIcon(ISystemBitmap* icon)
{
}

void FormGtk::maximize()
{
}

void FormGtk::minimize()
{
}

void FormGtk::restore()
{
}

bool FormGtk::isMaximized() const
{
	return false;
}

bool FormGtk::isMinimized() const
{
	return false;
}

void FormGtk::hideProgress()
{
}

void FormGtk::showProgress(int32_t current, int32_t total)
{
}

void FormGtk::setVisible(bool visible)
{
	gtk_widget_set_visible(m_window, visible);
	WidgetGtkImpl< IForm >::setVisible(visible);
}

gboolean FormGtk::signal_remove(GtkWidget* widget, GdkEvent* event, gpointer data)
{
	FormGtk* self = static_cast< FormGtk* >(data);
	T_FATAL_ASSERT(self != nullptr);

	CloseEvent closeEvent(self->m_owner);
	self->m_owner->raiseEvent(&closeEvent);
	if (closeEvent.consumed() && closeEvent.cancelled())
		return TRUE;

	return FALSE;
}

	}
}

