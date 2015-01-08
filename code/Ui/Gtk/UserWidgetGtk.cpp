#include "Ui/Gtk/UserWidgetGtk.h"

namespace traktor
{
	namespace ui
	{

UserWidgetGtk::UserWidgetGtk(EventSubject* owner)
:	WidgetGtkImpl< IUserWidget >(owner)
{
}

bool UserWidgetGtk::create(IWidget* parent, int style)
{
	Internal* parentInternal = static_cast< Internal* >(parent->getInternalHandle());
	T_FATAL_ASSERT(parentInternal);

	Gtk::Fixed* container = new Gtk::Fixed();
	parentInternal->container->put(*container, 0, 0);

	container->show();

	m_internal.container = container;
	m_internal.widget = container;

	return WidgetGtkImpl< IUserWidget >::create();
}

	}
}

