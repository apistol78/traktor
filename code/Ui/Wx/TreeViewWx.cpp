/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <wx/wx.h>
#include <wx/imaglist.h>
#include "Ui/Wx/TreeViewWx.h"
#include "Ui/Wx/TreeViewItemWx.h"
#include "Ui/Wx/TreeItemData.h"
#include "Ui/Wx/BitmapWx.h"
#include "Ui/TreeView.h"
#include "Ui/Events/TreeViewDragEvent.h"

namespace traktor
{
	namespace ui
	{

TreeViewWx::TreeViewWx(EventSubject* owner)
:	WidgetWxImpl< ITreeView, wxTreeCtrl >(owner)
,	m_allowDrag(false)
{
}

bool TreeViewWx::create(IWidget* parent, int style)
{
	long wxStyle = wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT;
	if (!(style & (WsClientBorder | WsBorder)))
		wxStyle |= wxNO_BORDER;
	if (style & WsClientBorder)
		wxStyle |= wxSUNKEN_BORDER;
	if (style & TreeView::WsAutoEdit)
		wxStyle |= wxTR_EDIT_LABELS;
	if (style & TreeView::WsDrag)
		m_allowDrag = true;

	m_window = new wxTreeCtrl();
	
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

	m_imageList = new wxImageList(16, 16);
	m_window->AssignImageList(m_imageList);

	if (!WidgetWxImpl< ITreeView, wxTreeCtrl >::create(style))
		return false;

	T_CONNECT(m_window, wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEvent, TreeViewWx, &TreeViewWx::onTreeItemActivated);
	T_CONNECT(m_window, wxEVT_COMMAND_TREE_SEL_CHANGED,    wxTreeEvent, TreeViewWx, &TreeViewWx::onTreeSelectionChanged);
	T_CONNECT(m_window, wxEVT_COMMAND_TREE_END_LABEL_EDIT, wxTreeEvent, TreeViewWx, &TreeViewWx::onTreeEndLabelEdit);
	T_CONNECT(m_window, wxEVT_COMMAND_TREE_BEGIN_DRAG,     wxTreeEvent, TreeViewWx, &TreeViewWx::onTreeBeginDrag);

	return true;
}

int TreeViewWx::addImage(ISystemBitmap* image, int imageCount)
{
	if (!image || imageCount <= 0)
		return -1;

	wxImage* wxi = static_cast< BitmapWx* >(image)->getWxImage();
	if (!wxi)
		return -1;
	
	return m_imageList->Add(*wxi, wxColour(255, 0, 0));
}

Ref< TreeViewItem > TreeViewWx::createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage)
{
	wxTreeItemId id = parent ?
		m_window->AppendItem(
			checked_type_cast< TreeViewItemWx* >(parent)->getId(),
			wstots(text).c_str(),
			image
		) :
		m_window->AddRoot(
			wstots(text).c_str(),
			image
		);
	T_ASSERT_M (id, L"Failed to create TreeView item");

	Ref< TreeViewItemWx > item = new TreeViewItemWx(m_window, parent, id);

	m_window->SetItemData(id, new TreeItemData(item));

	return item;
}

void TreeViewWx::removeItem(TreeViewItem* item)
{
	T_ASSERT (item);
	m_window->Delete(checked_type_cast< TreeViewItemWx* >(item)->getId());
}

void TreeViewWx::removeAllItems()
{
	m_window->DeleteAllItems();
}

Ref< TreeViewItem > TreeViewWx::getRootItem() const
{
	wxTreeItemId rootId = m_window->GetRootItem();
	if (!rootId)
		return 0;

	Ref< TreeViewItemWx > item = static_cast< TreeItemData* >(m_window->GetItemData(rootId))->getItem();
	T_ASSERT (item);

	return item;
}

Ref< TreeViewItem > TreeViewWx::getSelectedItem() const
{
	wxTreeItemId id = m_window->GetSelection();
	if (!id)
		return 0;

	Ref< TreeViewItemWx > item = static_cast< TreeItemData* >(m_window->GetItemData(id))->getItem();
	T_ASSERT (item);

	return item;
}

void TreeViewWx::onTreeItemActivated(wxTreeEvent& event)
{
	TreeViewItemActivateEvent activateEvent(m_owner, getSelectedItem());
	m_owner->raiseEvent(&activateEvent);
}

void TreeViewWx::onTreeSelectionChanged(wxTreeEvent& event)
{
	Ref< TreeViewItemWx > item = static_cast< TreeItemData* >(m_window->GetItemData(event.GetItem()))->getItem();
	T_ASSERT (item);

	SelectionChangeEvent selectionChangeEvent(m_owner);
	m_owner->raiseEvent(&selectionChangeEvent);
}

void TreeViewWx::onTreeEndLabelEdit(wxTreeEvent& event)
{
	if (event.IsEditCancelled())
		return;

	Ref< TreeViewItemWx > item = static_cast< TreeItemData* >(m_window->GetItemData(event.GetItem()))->getItem();
	T_ASSERT (item);

	std::wstring newLabel = tstows((const wxChar*)event.GetLabel());
	if (newLabel == item->getText())
		return;

	item->setText(newLabel);
	
	TreeViewContentChangeEvent contentChangeEvent(m_owner, item);
	m_owner->raiseEvent(&contentChangeEvent);
}

void TreeViewWx::onTreeBeginDrag(wxTreeEvent& event)
{
	if (!m_allowDrag)
		return;

	Ref< TreeViewItemWx > item = static_cast< TreeItemData* >(m_window->GetItemData(event.GetItem()))->getItem();
	T_ASSERT (item);

	TreeViewDragEvent dragEvent(m_owner, item, TreeViewDragEvent::DmDrag);
	m_owner->raiseEvent(&dragEvent);

	event.Allow();
}

	}
}
