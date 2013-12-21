#ifndef traktor_ui_ListViewGtk_H
#define traktor_ui_ListViewGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/IListView.h"

namespace traktor
{
	namespace ui
	{

class ListViewGtk : public WidgetGtkImpl< IListView >
{
public:
	ListViewGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual void setStyle(int style);

	virtual int addImage(IBitmap* image, int imageCount, bool smallImage);
	
	virtual void removeAllColumns();

	virtual void removeColumn(int columnIndex);

	virtual int getColumnCount() const;

	virtual int addColumn(const std::wstring& columnHeader, int width);

	virtual void setColumnHeader(int columnIndex, const std::wstring& columnHeader);

	virtual std::wstring getColumnHeader(int columnIndex) const;

	virtual void setColumnWidth(int columnIndex, int width);

	virtual int getColumnWidth(int columnIndex) const;

	virtual void setItems(ListViewItems* items);

	virtual Ref< ListViewItems > getItems() const;
	
	virtual int getSelectedItems(std::vector< int >& items) const;
};

	}
}

#endif	// traktor_ui_ListViewGtk_H

