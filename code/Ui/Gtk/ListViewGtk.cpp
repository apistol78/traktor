#include "Core/Misc/TString.h"
#include "Ui/Gtk/ListViewGtk.h"

namespace traktor
{
	namespace ui
	{

ListViewGtk::ListViewGtk(EventSubject* owner)
:	WidgetGtkImpl< IListView >(owner)
{
}

bool ListViewGtk::create(IWidget* parent, int style)
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

	return WidgetGtkImpl< IListView >::create();
}

void ListViewGtk::setStyle(int style)
{
}

int ListViewGtk::addImage(IBitmap* image, int imageCount, bool smallImage)
{
	return 0;
}

void ListViewGtk::removeAllColumns()
{
}

void ListViewGtk::removeColumn(int columnIndex)
{
}

int ListViewGtk::getColumnCount() const
{
	return 0;
}

int ListViewGtk::addColumn(const std::wstring& columnHeader, int width)
{
	return 0;
}

void ListViewGtk::setColumnHeader(int columnIndex, const std::wstring& columnHeader)
{
}

std::wstring ListViewGtk::getColumnHeader(int columnIndex) const
{
	return L"";
}

void ListViewGtk::setColumnWidth(int columnIndex, int width)
{
}

int ListViewGtk::getColumnWidth(int columnIndex) const
{
	return 0;
}

int ListViewGtk::getColumnFromPosition(int position) const
{
	return -1;
}

void ListViewGtk::setItems(ListViewItems* items)
{
}

Ref< ListViewItems > ListViewGtk::getItems() const
{
	return 0;
}

int ListViewGtk::getSelectedItems(std::vector< int >& items) const
{
	return 0;
}

	}
}

