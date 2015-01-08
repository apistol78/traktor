#include "Ui/Gtk/ListBoxGtk.h"

namespace traktor
{
	namespace ui
	{

ListBoxGtk::ListBoxGtk(EventSubject* owner)
:	WidgetGtkImpl< IListBox >(owner)
{
}

bool ListBoxGtk::create(IWidget* parent, int style)
{
	Internal* parentInternal = static_cast< Internal* >(parent->getInternalHandle());
	T_FATAL_ASSERT(parentInternal);

	Gtk::Fixed* container = new Gtk::Fixed();
	parentInternal->container->put(*container, 0, 0);

	Gtk::ScrolledWindow* scrolledWindow = new Gtk::ScrolledWindow();
	container->put(*scrolledWindow, 0, 0);
	scrolledWindow->show();

	m_internal.container = container;
	m_internal.widget = scrolledWindow;

	return WidgetGtkImpl< IListBox >::create();
}

int ListBoxGtk::add(const std::wstring& item)
{
	return 0;
}

bool ListBoxGtk::remove(int index)
{
	return false;
}

void ListBoxGtk::removeAll()
{
}

int ListBoxGtk::count() const
{
	return 0;
}

void ListBoxGtk::set(int index, const std::wstring& item)
{
}

std::wstring ListBoxGtk::get(int index) const
{
	return L"";
}

void ListBoxGtk::select(int index)
{
}

bool ListBoxGtk::selected(int index) const
{
	return false;
}

Rect ListBoxGtk::getItemRect(int index) const
{
	return Rect(0, 0, 0, 0);
}

	}
}

