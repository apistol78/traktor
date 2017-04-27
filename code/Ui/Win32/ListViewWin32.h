/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ListViewWin32_H
#define traktor_ui_ListViewWin32_H

#include "Ui/Itf/IListView.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class ListViewWin32 : public WidgetWin32Impl< IListView >
{
public:
	ListViewWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual void setStyle(int style);

	virtual int addImage(ISystemBitmap* image, int imageCount, bool smallImage);
	
	virtual void removeAllColumns();

	virtual void removeColumn(int columnIndex);

	virtual int getColumnCount() const;

	virtual int addColumn(const std::wstring& columnHeader, int width);

	virtual void setColumnHeader(int columnIndex, const std::wstring& columnHeader);

	virtual std::wstring getColumnHeader(int columnIndex) const;

	virtual void setColumnWidth(int columnIndex, int width);

	virtual int getColumnWidth(int columnIndex) const;

	virtual int getColumnFromPosition(int position) const;

	virtual void setItems(ListViewItems* items);

	virtual Ref< ListViewItems > getItems() const;
	
	virtual int getSelectedItems(std::vector< int >& items) const;

private:
	HIMAGELIST m_hImageList;
	int m_columnCount;
	Ref< ListViewItems > m_items;

	LRESULT eventReflectedNotify(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

	}
}

#endif	// traktor_ui_ListViewWin32_H
