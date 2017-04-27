/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ListViewCocoa_H
#define traktor_ui_ListViewCocoa_H

#import "Ui/Cocoa/NSListDataSource.h"

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/IListView.h"

namespace traktor
{
	namespace ui
	{

class ListViewCocoa
:	public WidgetCocoaImpl< IListView, NSTableView, NSScrollView >
,	public IListDataCallback
{
public:
	ListViewCocoa(EventSubject* owner);
	
	// IListView

	virtual bool create(IWidget* parent, int style) T_OVERRIDE T_FINAL;
	
	virtual void setStyle(int style) T_OVERRIDE T_FINAL;

	virtual int addImage(ISystemBitmap* image, int imageCount, bool smallImage) T_OVERRIDE T_FINAL;
	
	virtual void removeAllColumns() T_OVERRIDE T_FINAL;

	virtual void removeColumn(int columnIndex) T_OVERRIDE T_FINAL;

	virtual int getColumnCount() const T_OVERRIDE T_FINAL;

	virtual int addColumn(const std::wstring& columnHeader, int width) T_OVERRIDE T_FINAL;

	virtual void setColumnHeader(int columnIndex, const std::wstring& columnHeader) T_OVERRIDE T_FINAL;

	virtual std::wstring getColumnHeader(int columnIndex) const T_OVERRIDE T_FINAL;

	virtual void setColumnWidth(int columnIndex, int width) T_OVERRIDE T_FINAL;

	virtual int getColumnWidth(int columnIndex) const T_OVERRIDE T_FINAL;

	virtual int getColumnFromPosition(int position) const T_OVERRIDE T_FINAL;

	virtual void setItems(ListViewItems* items) T_OVERRIDE T_FINAL;

	virtual Ref< ListViewItems > getItems() const T_OVERRIDE T_FINAL;
	
	virtual int getSelectedItems(std::vector< int >& items) const T_OVERRIDE T_FINAL;
	
	// IListDataCallback
	
	virtual int listCount() const T_OVERRIDE T_FINAL;
	
	virtual std::wstring listValue(NSTableColumn* tableColumn, int index) const T_OVERRIDE T_FINAL;
	
private:
	Ref< ListViewItems > m_items;
};

	}
}

#endif	// traktor_ui_ListViewCocoa_H
