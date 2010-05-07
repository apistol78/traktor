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
	// @fixme Not safe, we must check so the internal handle in fact are a Gtk::Fixed container. Also need to ensure cleanup of m_parentContainer.
	m_parentContainer = static_cast< Gtk::Fixed* >(parent->getInternalHandle());
	if (!m_parentContainer)
		return false;

	Gtk::ScrolledWindow* scrolledWindow = new Gtk::ScrolledWindow();

	m_parentContainer->put(*scrolledWindow, 0, 0);

	scrolledWindow->show();

	m_widget = scrolledWindow;

	return true;
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

