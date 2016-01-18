#include "Ui/ListView.h"
#include "Ui/ListViewItem.h"
#include "Ui/ListViewItems.h"
#include "Ui/Win32/ListViewWin32.h"
#include "Ui/Win32/BitmapWin32.h"

namespace traktor
{
	namespace ui
	{

ListViewWin32::ListViewWin32(EventSubject* owner)
:	WidgetWin32Impl< IListView >(owner)
,	m_hImageList(NULL)
,	m_columnCount(0)
{
}

bool ListViewWin32::create(IWidget* parent, int style)
{
	UINT nativeStyle, nativeStyleEx;
	getNativeStyles(style, nativeStyle, nativeStyleEx);

	if (style & ListView::WsList)
	{
		nativeStyle |= LVS_LIST;
		nativeStyleEx |= LVS_EX_FULLROWSELECT;
	}
	else if (style & ListView::WsReport)
	{
		nativeStyle |= LVS_REPORT;
		nativeStyleEx |= LVS_EX_FULLROWSELECT;
	}
	else if (style & ListView::WsIconNormal)
		nativeStyle |= LVS_ICON | LVS_AUTOARRANGE;
	else if (style & ListView::WsIconSmall)
		nativeStyle |= LVS_SMALLICON | LVS_AUTOARRANGE;

	if (style & ListView::WsSelectSingle)
		nativeStyle |= LVS_SINGLESEL;

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		WC_LISTVIEW,
		_T(""),
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | LVS_SHOWSELALWAYS | nativeStyle,
		nativeStyleEx,
		0,
		0,
		0,
		0,
		0,
		true
	))
		return false;

	ListView_SetExtendedListViewStyle(m_hWnd, nativeStyleEx);

	if (!WidgetWin32Impl::create(style))
		return false;

	m_hWnd.registerMessageHandler(WM_REFLECTED_NOTIFY, new MethodMessageHandler< ListViewWin32 >(this, &ListViewWin32::eventReflectedNotify));

	return true;
}

void ListViewWin32::setStyle(int style)
{
	UINT nativeStyle, nativeStyleEx;
	getNativeStyles(style, nativeStyle, nativeStyleEx);

	if (style & ListView::WsList)
		nativeStyle |= LVS_LIST;
	else if (style & ListView::WsReport)
		nativeStyle |= LVS_REPORT;
	else if (style & ListView::WsIconNormal)
		nativeStyle |= LVS_ICON | LVS_AUTOARRANGE;
	else if (style & ListView::WsIconSmall)
		nativeStyle |= LVS_SMALLICON | LVS_AUTOARRANGE;

	SetWindowLong(m_hWnd, GWL_STYLE, WS_VISIBLE | WS_CHILD | WS_TABSTOP | LVS_SINGLESEL | LVS_SHOWSELALWAYS | nativeStyle);
	SetWindowLong(m_hWnd, GWL_EXSTYLE, nativeStyleEx);

	UpdateWindow(m_hWnd);
}

int ListViewWin32::addImage(ISystemBitmap* image, int imageCount, bool smallImage)
{
	if (!m_hImageList)
	{
		int dim = image->getSize().cy;
		m_hImageList = ImageList_Create(dim, dim, ILC_COLOR24 | ILC_MASK, 0, 0);
		ListView_SetImageList(m_hWnd, m_hImageList, LVSIL_NORMAL);
		ListView_SetImageList(m_hWnd, m_hImageList, LVSIL_SMALL);
	}

	HBITMAP hImageBitmap = reinterpret_cast< BitmapWin32* >(image)->getHBitmap();
	if (!hImageBitmap)
		return -1;

	ImageList_AddMasked(m_hImageList, hImageBitmap, reinterpret_cast< BitmapWin32* >(image)->getMask());

	return 0;
}

void ListViewWin32::removeAllColumns()
{
	ListView_DeleteAllItems(m_hWnd);
}

void ListViewWin32::removeColumn(int columnIndex)
{
	ListView_DeleteColumn(m_hWnd, columnIndex);
}

int ListViewWin32::getColumnCount() const
{
	return m_columnCount;
}

int ListViewWin32::addColumn(const std::wstring& columnHeader, int width)
{
	tstring tmp = wstots(columnHeader);
	LVCOLUMN lvc;

	std::memset(&lvc, 0, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = m_columnCount;
	lvc.pszText = const_cast< LPTSTR >(tmp.c_str());
	lvc.cx = width;
	lvc.fmt = LVCFMT_LEFT;
	ListView_InsertColumn(m_hWnd, m_columnCount, &lvc);

	return m_columnCount++;
}

void ListViewWin32::setColumnHeader(int columnIndex, const std::wstring& columnHeader)
{
	tstring tmp = wstots(columnHeader);
	LVCOLUMN lvc;

	std::memset(&lvc, 0, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = columnIndex;
	lvc.pszText = const_cast< LPTSTR >(tmp.c_str());
	lvc.fmt = LVCFMT_LEFT;
	ListView_SetColumn(m_hWnd, columnIndex, &lvc);
}

std::wstring ListViewWin32::getColumnHeader(int columnIndex) const
{
	TCHAR tmp[256];
	LVCOLUMN lvc;
	
	lvc.mask = LVCF_TEXT;
	lvc.pszText = tmp;
	lvc.cchTextMax = sizeof(tmp) / sizeof(TCHAR);
	if (ListView_GetColumn(m_hWnd, columnIndex, &lvc) == FALSE)
		return L"";

	return tstows(tmp);
}

void ListViewWin32::setColumnWidth(int columnIndex, int width)
{
	ListView_SetColumnWidth(m_hWnd, columnIndex, width); 
}

int ListViewWin32::getColumnWidth(int columnIndex) const
{
	return ListView_GetColumnWidth(m_hWnd, columnIndex);
}

int ListViewWin32::getColumnFromPosition(int position) const
{
	POINT origin = { 0 };
	ListView_GetOrigin(m_hWnd, &origin);

	int left = -origin.x;
	for (int i = 0; i < m_columnCount; ++i)
	{
		int width = ListView_GetColumnWidth(m_hWnd, i);
		if (position >= left && position <= left + width)
			return i;
		left += width;
	}

	return -1;
}

void ListViewWin32::setItems(ListViewItems* items)
{
	if (items)
	{
		int32_t currentCount = m_items ? m_items->count() : 0;
		int32_t nextCount = items->count();

		// Remove trailing rows if too many.
		while (nextCount < currentCount)
			ListView_DeleteItem(m_hWnd, --currentCount);

		// Modify existing items.
		T_ASSERT (currentCount <= nextCount);
		for (int32_t i = 0; i < currentCount; ++i)
		{
			Ref< ListViewItem > item = items->get(i);
			for (int j = 0; j < item->getColumnCount(); ++j)
			{
				tstring tmp = wstots(item->getText(j));

				LV_ITEM lvi;

				std::memset(&lvi, 0, sizeof(lvi));
				lvi.mask = LVIF_TEXT | LVIF_IMAGE;
				lvi.pszText = const_cast< LPTSTR >(tmp.c_str());
				lvi.iImage = item->getImage(j);
				lvi.iItem = i;
				lvi.iSubItem = j;

				ListView_SetItem(m_hWnd, &lvi);
			}
		}

		// Add trailing rows if we got more than before.
		for (int32_t i = currentCount; i < nextCount; ++i)
		{
			Ref< ListViewItem > item = items->get(i);
			for (int j = 0; j < item->getColumnCount(); ++j)
			{
				tstring tmp = wstots(item->getText(j));

				LV_ITEM lvi;

				std::memset(&lvi, 0, sizeof(lvi));
				lvi.mask = LVIF_TEXT | LVIF_IMAGE;
				lvi.pszText = const_cast< LPTSTR >(tmp.c_str());
				lvi.iImage = item->getImage(j);
				lvi.iItem = i;
				lvi.iSubItem = j;

				if (j == 0)
					ListView_InsertItem(m_hWnd, &lvi);
				else
					ListView_SetItem(m_hWnd, &lvi);
			}
		}
	}
	else
		ListView_DeleteAllItems(m_hWnd);

	m_items = items;
}

Ref< ListViewItems > ListViewWin32::getItems() const
{
	return m_items;
}

int ListViewWin32::getSelectedItems(std::vector< int >& items) const
{
	int index = ListView_GetNextItem(m_hWnd, -1, LVNI_SELECTED);
	while (index >= 0)
	{
		items.push_back(index);
		index = ListView_GetNextItem(m_hWnd, index, LVNI_SELECTED);
	}
	return int(items.size());
}

LRESULT ListViewWin32::eventReflectedNotify(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	LPNMHDR nmhdr = reinterpret_cast< LPNMHDR >(lParam);
	if (nmhdr->code == LVN_ITEMCHANGED)
	{
		SelectionChangeEvent selectionChangeEvent(m_owner);
		m_owner->raiseEvent(&selectionChangeEvent);
		pass = false;
	}
	else if (nmhdr->code == LVN_ITEMACTIVATE)
	{
		LPNMITEMACTIVATE nmia = reinterpret_cast< LPNMITEMACTIVATE >(nmhdr);

		Ref< ListViewItem > item = m_items->get(nmia->iItem);
		T_ASSERT (item != 0);

		ListViewItemActivateEvent activateEvent(m_owner, item);
		m_owner->raiseEvent(&activateEvent);
		pass = false;
	}
	return 0;
}

	}
}
