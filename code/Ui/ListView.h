/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ListView_H
#define traktor_ui_ListView_H

#include "Ui/ListViewItems.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Bitmap;

/*! \brief List view.
 * \ingroup UI
 */
class T_DLLCLASS ListView : public Widget
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsList				= WsUser,
		WsReport			= WsUser << 1,
		WsIconNormal		= WsUser << 2,
		WsIconSmall			= WsUser << 3,
		WsSelectSingle		= 0,
		WsSelectMultiple	= WsUser << 4
	};
	
	bool create(Widget* parent, int style);

	void setStyle(int style);
	
	int addImage(Bitmap* image, int imageCount, bool smallImage = false);
	
	void removeAllColumns();

	void removeColumn(int columnIndex);

	int getColumnCount() const;

	int addColumn(const std::wstring& columnHeader, int width);

	void setColumnHeader(int columnIndex, const std::wstring& columnHeader);

	std::wstring getColumnHeader(int columnIndex) const;

	void setColumnWidth(int columnIndex, int width);

	int getColumnWidth(int columnIndex) const;

	int getColumnFromPosition(int position) const;

	void setItems(ListViewItems* items);

	Ref< ListViewItems > getItems() const;
	
	Ref< ListViewItem > getSelectedItem() const;

	int getSelectedItems(std::vector< int >& items) const;
};

	}
}

#endif	// traktor_ui_ListView_H
