#ifndef traktor_ui_PopupMenuGtk_H
#define traktor_ui_PopupMenuGtk_H

#include <gtk/gtk.h>
#include "Ui/Itf/IPopupMenu.h"

namespace traktor
{
	namespace ui
	{

class PopupMenuGtk : public IPopupMenu
{
public:
	PopupMenuGtk();
	
	virtual bool create();

	virtual void destroy();

	virtual void add(MenuItem* item);

	virtual MenuItem* show(IWidget* parent, const Point& at);

private:
	GtkWidget* m_menu;
	Point m_at;
	MenuItem* m_selected;

	static void menuPosition(GtkMenu* menu, gint* x, gint* y, gint* pushIn, gpointer userData);

	static void signal_cancel(GtkMenuShell* menu, gpointer userData);

	static void signal_activate(GtkMenuItem* item, gpointer userData);
};

	}
}

#endif	// traktor_ui_PopupMenuGtk_H
