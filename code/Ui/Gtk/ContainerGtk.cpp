#include "Core/Log/Log.h"
#include "Ui/Gtk/BareContainer.h"
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
	Warp* p = static_cast< Warp* >(parent->getInternalHandle());
	T_FATAL_ASSERT(p != nullptr);

	m_warp.widget = t_bare_container_new();
	gtk_container_add(GTK_CONTAINER(p->widget), GTK_WIDGET(m_warp.widget));

	return WidgetGtkImpl< IContainer >::create(parent);
}

	}
}

