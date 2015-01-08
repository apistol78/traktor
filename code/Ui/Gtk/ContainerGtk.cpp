#include "Ui/Gtk/ContainerGtk.h"

namespace traktor
{
	namespace ui
	{

ContainerGtk::ContainerGtk(EventSubject* owner)
:	WidgetGtkImpl< IContainer >(owner)
{
}

bool ContainerGtk::create(IWidget* parent, int style)
{
	Internal* parentInternal = static_cast< Internal* >(parent->getInternalHandle());
	T_FATAL_ASSERT(parentInternal);

	Gtk::Fixed* container = new Gtk::Fixed();
	parentInternal->container->put(*container, 0, 0);

	container->show();

	m_internal.container = container;
	m_internal.widget = container;

	return WidgetGtkImpl< IContainer >::create();
}

	}
}

