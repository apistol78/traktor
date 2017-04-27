/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Wx/ListViewWx.h"
#include "Ui/Wx/BitmapWx.h"
#include "Ui/ListView.h"
#include "Ui/ListViewItem.h"
#include "Ui/ListViewItems.h"

namespace traktor
{
	namespace ui
	{

ListViewWx::ListViewWx(EventSubject* owner)
:	WidgetWxImpl< IListView, wxListCtrl >(owner)
{
}

bool ListViewWx::create(IWidget* parent, int style)
{
	if (!parent)
		return false;
	
	int wxStyle = wxLC_AUTOARRANGE;
	if (style & ListView::WsList)
		wxStyle |= wxLC_LIST;
	else if (style & ListView::WsReport)
		wxStyle |= wxLC_REPORT;
	else if (style & ListView::WsIconNormal)
		wxStyle |= wxLC_ICON;
	else if (style & ListView::WsIconSmall)
		wxStyle |= wxLC_SMALL_ICON;
	else if (!(style & ListView::WsSelectMultiple))
		wxStyle |= wxLC_SINGLE_SEL;

	m_window = new wxListCtrl();

	if (!m_window->Create(
		static_cast< wxWindow* >(parent->getInternalHandle()),
		-1,
		wxDefaultPosition,
		wxDefaultSize,
		wxStyle 
	))
	{
		m_window->Destroy();
		return false;
	}
	
	m_imageListNormal.Create(24, 24);
	m_window->SetImageList(&m_imageListNormal, wxIMAGE_LIST_NORMAL);

	m_imageListSmall.Create(16, 16);
	m_window->SetImageList(&m_imageListSmall, wxIMAGE_LIST_SMALL);

	if (!WidgetWxImpl< IListView, wxListCtrl >::create(style))
		return false;
	
	T_CONNECT(m_window, wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEvent, ListViewWx, &ListViewWx::onSelected);
	T_CONNECT(m_window, wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEvent, ListViewWx, &ListViewWx::onActivated);

	return true;
}

int ListViewWx::addImage(ISystemBitmap* image, int imageCount, bool smallImage)
{
	if (!image || imageCount <= 0)
		return -1;

	wxImage* wxi = static_cast< BitmapWx* >(image)->getWxImage();
	if (!wxi)
		return -1;
	
	return (smallImage ? m_imageListSmall : m_imageListNormal).Add(*wxi);
}

void ListViewWx::removeAllColumns()
{
	while (getColumnCount() > 0)
		removeColumn(0);
}

void ListViewWx::removeColumn(int columnIndex)
{
	m_window->DeleteColumn(columnIndex);
}

int ListViewWx::getColumnCount() const
{
	return m_window->GetColumnCount();
}

int ListViewWx::addColumn(const std::wstring& columnHeader, int width)
{
	int id = getColumnCount();
	m_window->InsertColumn(id, wstots(columnHeader).c_str(), wxLIST_FORMAT_LEFT, width);
	return id;
}

void ListViewWx::setColumnHeader(int columnIndex, const std::wstring& columnHeader)
{
	wxListItem listItem;
	listItem.SetMask(wxLIST_MASK_TEXT);
	listItem.SetText(wstots(columnHeader).c_str());
	m_window->SetColumn(columnIndex, listItem);
}

std::wstring ListViewWx::getColumnHeader(int columnIndex) const
{
	return L"";
}

void ListViewWx::setColumnWidth(int columnIndex, int width)
{
	m_window->SetColumnWidth(columnIndex, width);
}

int ListViewWx::getColumnWidth(int columnIndex) const
{
	return m_window->GetColumnWidth(columnIndex);
}

int ListViewWx::getColumnFromPosition(int position) const
{
	return -1;
}

void ListViewWx::setItems(ListViewItems* items)
{
	m_window->DeleteAllItems();
	if ((m_items = items) != 0)
	{
		for (int i = 0; i < items->count(); ++i)
		{
			Ref< ListViewItem > item = items->get(i);
			for (int j = 0; j < item->getColumnCount(); ++j)
			{
				if (j == 0)
				{
					m_window->InsertItem(
						i,
						wstots(item->getText(0)).c_str(),
						item->getImage(0)
					);
				}
				else
				{
					m_window->SetItem(
						i,
						j,
						wstots(item->getText(j)).c_str(),
						item->getImage(j)
					);
				}
			}
		}
	}
}

Ref< ListViewItems > ListViewWx::getItems() const
{
	return m_items;
}

int ListViewWx::getSelectedItems(std::vector< int >& items) const
{
	items.resize(0);
	long item = -1;
	for (;;)
	{
		item = m_window->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item <= -1)
			break;
		items.push_back(item);
	}
	return int(items.size());
}

void ListViewWx::onSelected(wxListEvent& event)
{
	SelectionChangeEvent selectionChangeEvent(m_owner);
	m_owner->raiseEvent(&selectionChangeEvent);
}

void ListViewWx::onActivated(wxListEvent& event)
{
	ListViewItemActivateEvent activateEvent(m_owner, m_items->get(event.GetIndex()));
	m_owner->raiseEvent(&activateEvent);
}

	}
}
