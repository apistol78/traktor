#include "Core/Log/Log.h"
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
	Warp* p = static_cast< Warp* >(parent->getInternalHandle());
	T_FATAL_ASSERT(p != nullptr);

	m_warp.widget = t_bare_container_new();
	gtk_container_add(GTK_CONTAINER(p->widget), m_warp.widget);

	return WidgetGtkImpl< IUserWidget >::create(parent);
}

	}
}

