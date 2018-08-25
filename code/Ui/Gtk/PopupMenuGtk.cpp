#include "Core/Log/Log.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Gtk/PopupMenuGtk.h"
#include "Ui/Gtk/Warp.h"

namespace traktor
{
	namespace ui
	{

PopupMenuGtk::PopupMenuGtk()
:	m_menu(nullptr)
{
}

bool PopupMenuGtk::create()
{
	m_menu = gtk_menu_new();
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
	GtkWidget* mi = gtk_menu_item_new_with_label("Label");
	gtk_menu_shell_append(GTK_MENU_SHELL(m_menu), mi);
}

MenuItem* PopupMenuGtk::show(IWidget* parent, const Point& at)
{
	Warp* pr = static_cast< Warp* >(parent->getInternalHandle());
	T_FATAL_ASSERT(pr != nullptr);

	gtk_menu_popup_at_pointer(GTK_MENU(m_menu), nullptr);

	return 0;
}

	}
}
