#ifndef traktor_ui_IListView_H
#define traktor_ui_IListView_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

class ISystemBitmap;

class ListViewItems;
class ListViewItem;

/*! \brief ListView interface.
 * \ingroup UI
 */
class IListView : public IWidget
{
public:
	virtual bool create(IWidget* parent, int style) = 0;

	virtual void setStyle(int style) = 0;

	virtual int addImage(ISystemBitmap* image, int imageCount, bool smallImage) = 0;
	
	virtual void removeAllColumns() = 0;

	virtual void removeColumn(int columnIndex) = 0;

	virtual int getColumnCount() const = 0;

	virtual int addColumn(const std::wstring& columnHeader, int width) = 0;

	virtual void setColumnHeader(int columnIndex, const std::wstring& columnHeader) = 0;

	virtual std::wstring getColumnHeader(int columnIndex) const = 0;

	virtual void setColumnWidth(int columnIndex, int width) = 0;

	virtual int getColumnWidth(int columnIndex) const = 0;

	virtual int getColumnFromPosition(int position) const = 0;

	virtual void setItems(ListViewItems* items) = 0;

	virtual Ref< ListViewItems > getItems() const = 0;
	
	virtual int getSelectedItems(std::vector< int >& items) const = 0;
};

	}
}

#endif	// traktor_ui_IListView_H
