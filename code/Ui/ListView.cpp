/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/ListView.h"
#include "Ui/ListViewItem.h"
#include "Ui/Itf/IListView.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ListView", ListView, Widget)

bool ListView::create(Widget* parent, int style)
{
	if (!parent)
		return false;

	IListView* listView = Application::getInstance()->getWidgetFactory()->createListView(this);
	if (!listView)
	{
		log::error << L"Failed to create native widget peer (ListView)" << Endl;
		return false;
	}

	if (!listView->create(parent->getIWidget(), style))
	{
		listView->destroy();
		return false;
	}

	m_widget = listView;
	
	return Widget::create(parent);
}

void ListView::setStyle(int style)
{
	T_ASSERT (m_widget);
	static_cast< IListView* >(m_widget)->setStyle(style);
}

int ListView::addImage(Bitmap* image, int imageCount, bool smallImage)
{
	T_ASSERT (m_widget);
	if (!image || !image->getSystemBitmap())
		return -1;
	return static_cast< IListView* >(m_widget)->addImage(image->getSystemBitmap(), imageCount, smallImage);
}

void ListView::removeAllColumns()
{
	T_ASSERT (m_widget);
	static_cast< IListView* >(m_widget)->removeAllColumns();
}

void ListView::removeColumn(int columnIndex)
{
	T_ASSERT (m_widget);
	static_cast< IListView* >(m_widget)->removeColumn(columnIndex);
}

int ListView::getColumnCount() const
{
	T_ASSERT (m_widget);
	return static_cast< IListView* >(m_widget)->getColumnCount();
}

int ListView::addColumn(const std::wstring& columnHeader, int width)
{
	T_ASSERT (m_widget);
	return static_cast< IListView* >(m_widget)->addColumn(columnHeader, width);
}

void ListView::setColumnHeader(int columnIndex, const std::wstring& columnHeader)
{
	T_ASSERT (m_widget);
	static_cast< IListView* >(m_widget)->setColumnHeader(columnIndex, columnHeader);
}

std::wstring ListView::getColumnHeader(int columnIndex) const
{
	T_ASSERT (m_widget);
	return static_cast< IListView* >(m_widget)->getColumnHeader(columnIndex);
}

void ListView::setColumnWidth(int columnIndex, int width)
{
	T_ASSERT (m_widget);
	static_cast< IListView* >(m_widget)->setColumnWidth(columnIndex, width);
}

int ListView::getColumnWidth(int columnIndex) const
{
	T_ASSERT (m_widget);
	return static_cast< IListView* >(m_widget)->getColumnWidth(columnIndex);
}

int ListView::getColumnFromPosition(int position) const
{
	T_ASSERT (m_widget);
	return static_cast< IListView* >(m_widget)->getColumnFromPosition(position);
}

void ListView::setItems(ListViewItems* items)
{
	T_ASSERT (m_widget);
	static_cast< IListView* >(m_widget)->setItems(items);
}

Ref< ListViewItems > ListView::getItems() const
{
	T_ASSERT (m_widget);
	return static_cast< IListView* >(m_widget)->getItems();
}

Ref< ListViewItem > ListView::getSelectedItem() const
{
	T_ASSERT (m_widget);
	std::vector< int > selectedItems;
	return (getSelectedItems(selectedItems) > 0) ? getItems()->get(selectedItems.front()) : 0;
}

int ListView::getSelectedItems(std::vector< int >& items) const
{
	T_ASSERT (m_widget);
	return static_cast< IListView* >(m_widget)->getSelectedItems(items);
}

	}
}
