#include "Core/Misc/TString.h"
#include "Ui/Gtk/StaticGtk.h"

namespace traktor
{
	namespace ui
	{

StaticGtk::StaticGtk(EventSubject* owner)
:	WidgetGtkImpl< IStatic >(owner)
{
}

bool StaticGtk::create(IWidget* parent, const std::wstring& text)
{
	Internal* parentInternal = static_cast< Internal* >(parent->getInternalHandle());
	T_FATAL_ASSERT(parentInternal);

	Gtk::Fixed* container = new Gtk::Fixed();
	parentInternal->container->put(*container, 0, 0);

	Gtk::Label* label = new Gtk::Label(wstombs(text).c_str());
	container->put(*label, 0, 0);

	m_internal.container = container;
	m_internal.widget = label;

	return WidgetGtkImpl< IStatic >::create();
}

	}
}

