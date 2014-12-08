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
	// @fixme Not safe, we must check so the internal handle in fact are a Gtk::Fixed container. Also need to ensure cleanup of m_parentContainer.
	m_parentContainer = static_cast< Gtk::Fixed* >(parent->getInternalHandle());
	if (!m_parentContainer)
		return false;

	Gtk::ScrolledWindow* scrolledWindow = new Gtk::ScrolledWindow();

	m_parentContainer->put(*scrolledWindow, 0, 0);

	m_widget = scrolledWindow;

	return true;
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

