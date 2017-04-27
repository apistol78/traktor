/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ListViewWx_H
#define traktor_ui_ListViewWx_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IListView.h"

namespace traktor
{
	namespace ui
	{

class ListViewWx : public WidgetWxImpl< IListView, wxListCtrl >
{
public:
	ListViewWx(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

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
	Ref< ListViewItems > m_items;
	wxImageList m_imageListNormal;
	wxImageList m_imageListSmall;
	
	void onSelected(wxListEvent& event);
	
	void onActivated(wxListEvent& event);
};

	}
}

#endif	// traktor_ui_ListViewWx_H
