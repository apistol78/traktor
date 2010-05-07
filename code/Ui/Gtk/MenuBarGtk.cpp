#include "Ui/Gtk/MenuBarGtk.h"

namespace traktor
{
	namespace ui
	{

bool MenuBarGtk::create(IForm* form)
{
	return true;
}

void MenuBarGtk::destroy()
{
	delete this;
}

void MenuBarGtk::add(MenuItem* item)
{
}

	}
}
