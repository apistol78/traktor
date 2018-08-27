#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/MenuItem.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Gtk/PopupMenuGtk.h"
#include "Ui/Gtk/Warp.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

struct ItemData
{
	PopupMenuGtk* self;
	MenuItem* item;

	ItemData(PopupMenuGtk* self_, MenuItem* item_)
	:	self(self_)
	,	item(item_)
	{
	}
};

		}

PopupMenuGtk::PopupMenuGtk()
:	m_menu(nullptr)
,	m_selected(nullptr)
{
}

bool PopupMenuGtk::create()
{
	m_menu = gtk_menu_new();
	g_signal_connect(m_menu, "cancel", G_CALLBACK(PopupMenuGtk::signal_cancel), this);
	return true;
}

void PopupMenuGtk::destroy()
{
	if (m_menu != nullptr)
	{
		gtk_widget_destroy(m_menu);
		m_menu = nullptr;
	}
}

void PopupMenuGtk::add(MenuItem* item)
{
	GtkWidget* mi = gtk_menu_item_new_with_label(wstombs(item->getText()).c_str());
	g_signal_connect(mi, "activate", G_CALLBACK(PopupMenuGtk::signal_activate), new ItemData(this, item));
	gtk_menu_shell_append(GTK_MENU_SHELL(m_menu), mi);
}

MenuItem* PopupMenuGtk::show(IWidget* parent, const Point& at)
{
	Warp* pr = static_cast< Warp* >(parent->getInternalHandle());
	T_FATAL_ASSERT(pr != nullptr);

	GtkWindow* parentWindow = GTK_WINDOW(gtk_widget_get_toplevel(pr->widget));
	if (parentWindow == nullptr)
	{
		log::error << L"Unable to get toplevel window from parent widget." << Endl;
		return nullptr;
	}

	gint px = 0, py = 0;
	gtk_window_get_position(parentWindow, &px, &py);

	m_at = at + Size(px, py);
	m_selected = nullptr;

	gtk_widget_show_all(m_menu);
	gtk_menu_popup(
		GTK_MENU(m_menu),
		nullptr,
		nullptr,
		(GtkMenuPositionFunc)&PopupMenuGtk::menuPosition,
		this,
		0,
		gtk_get_current_event_time()
	);

	// Run an internal loop until popup menu has been confirmed.
	gtk_main();
	return m_selected;
}

void PopupMenuGtk::menuPosition(GtkMenu* menu, gint* x, gint* y, gint* pushIn, gpointer userData)
{
	PopupMenuGtk* self = reinterpret_cast< PopupMenuGtk* >(userData);
	*x = self->m_at.x;
	*y = self->m_at.y;
}

void PopupMenuGtk::signal_cancel(GtkMenuShell* menu, gpointer userData)
{
	gtk_main_quit();
}

void PopupMenuGtk::signal_activate(GtkMenuItem* item, gpointer userData)
{
	ItemData* id = reinterpret_cast< ItemData* >(userData);

	gtk_main_quit();

	id->self->m_selected = id->item;
}

	}
}
