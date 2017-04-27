/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_TreeViewWx_H
#define traktor_ui_TreeViewWx_H

#include <wx/wx.h>
#include <wx/treectrl.h>
#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/ITreeView.h"

namespace traktor
{
	namespace ui
	{

class TreeViewWx : public WidgetWxImpl< ITreeView, wxTreeCtrl >
{
public:
	TreeViewWx(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual int addImage(ISystemBitmap* image, int imageCount);

	virtual Ref< TreeViewItem > createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage);

	virtual void removeItem(TreeViewItem* item);

	virtual void removeAllItems();

	virtual Ref< TreeViewItem > getRootItem() const;

	virtual Ref< TreeViewItem > getSelectedItem() const;

private:
	wxImageList* m_imageList;
	Ref< TreeViewItem > m_rootItem;
	bool m_allowDrag;

	void updateItems(const wxTreeItemId& parentId, TreeViewItem* item);

	void onTreeItemActivated(wxTreeEvent& event);

	void onTreeSelectionChanged(wxTreeEvent& event);

	void onTreeEndLabelEdit(wxTreeEvent& event);

	void onTreeBeginDrag(wxTreeEvent& event);
};

	}
}

#endif	// traktor_ui_TreeViewWx_H
