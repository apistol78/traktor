/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_TreeViewWin32_H
#define traktor_ui_TreeViewWin32_H

#include "Ui/Itf/ITreeView.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

class TreeViewItemWin32;

/*! \brief
 * \ingroup UIW32
 */
class TreeViewWin32 : public WidgetWin32Impl< ITreeView >
{
public:
	TreeViewWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual void destroy();

	virtual int addImage(ISystemBitmap* image, int imageCount);

	virtual Ref< TreeViewItem > createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage);

	virtual void removeItem(TreeViewItem* item);

	virtual void removeAllItems();

	virtual Ref< TreeViewItem > getRootItem() const;

	virtual Ref< TreeViewItem > getSelectedItem() const;

private:
	friend class TreeViewItemWin32;

	HIMAGELIST m_hImageList;
	std::map< HTREEITEM, Ref< TreeViewItemWin32 > > m_items;
	Ref< TreeViewItemWin32 > m_dragItem;
	TCHAR m_editBuffer[1024];

	Ref< TreeViewItemWin32 > getFromHandle(HTREEITEM hItem) const;

	LRESULT eventButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventNotify(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

	}
}

#endif	// traktor_ui_TreeViewWin32_H
