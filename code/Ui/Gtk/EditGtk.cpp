#include "Core/Misc/TString.h"
#include "Ui/Gtk/EditGtk.h"

namespace traktor
{
	namespace ui
	{

EditGtk::EditGtk(EventSubject* owner)
:	WidgetGtkImpl< IEdit >(owner)
{
}

bool EditGtk::create(IWidget* parent, const std::wstring& text, int style)
{
	Internal* parentInternal = static_cast< Internal* >(parent->getInternalHandle());
	T_FATAL_ASSERT(parentInternal);

	Gtk::Fixed* container = new Gtk::Fixed();
	parentInternal->container->put(*container, 0, 0);

	Gtk::Entry* entry = new Gtk::Entry();
	container->put(*entry, 0, 0);
	entry->show();

	m_internal.container = container;
	m_internal.widget = entry;

	return WidgetGtkImpl< IEdit >::create();
}

void EditGtk::setSelection(int from, int to)
{
}

void EditGtk::getSelection(int& outFrom, int& outTo) const
{
}

void EditGtk::selectAll()
{
}

	}
}

