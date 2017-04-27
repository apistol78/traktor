/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Wx/TreeViewItemWx.h"
#include "Ui/Wx/TreeItemData.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewItemWx", TreeViewItemWx, TreeViewItem)

TreeViewItemWx::TreeViewItemWx(wxTreeCtrl* treeCtrl, TreeViewItem* parent, wxTreeItemId id)
:	m_treeCtrl(treeCtrl)
,	m_parent(parent)
,	m_id(id)
{
}
	
TreeViewItemWx::~TreeViewItemWx()
{
}
	
void TreeViewItemWx::setText(const std::wstring& text)
{
	m_treeCtrl->SetItemText(m_id, wstots(text).c_str());
}
	
std::wstring TreeViewItemWx::getText() const
{
	return tstows((const wxChar*)m_treeCtrl->GetItemText(m_id));
}

void TreeViewItemWx::setBold(bool bold)
{
}

bool TreeViewItemWx::isBold() const
{
	return false;
}

void TreeViewItemWx::setImage(int image)
{
	m_treeCtrl->SetItemImage(m_id, image);
}
	
int TreeViewItemWx::getImage() const
{
	return m_treeCtrl->GetItemImage(m_id);
}
	
void TreeViewItemWx::setExpandedImage(int expandedImage)
{
	m_treeCtrl->SetItemImage(m_id, expandedImage, wxTreeItemIcon_Expanded);
}
	
int TreeViewItemWx::getExpandedImage() const
{
	return m_treeCtrl->GetItemImage(m_id, wxTreeItemIcon_Expanded);
}
	
bool TreeViewItemWx::isExpanded() const
{
	return m_treeCtrl->IsExpanded(m_id);
}
	
void TreeViewItemWx::expand()
{
	m_treeCtrl->Expand(m_id);
}
	
bool TreeViewItemWx::isCollapsed() const
{
	return !m_treeCtrl->IsExpanded(m_id);
}
	
void TreeViewItemWx::collapse()
{
	m_treeCtrl->Collapse(m_id);
}
	
bool TreeViewItemWx::isSelected() const
{
	return m_treeCtrl->IsSelected(m_id);
}
	
void TreeViewItemWx::select()
{
	m_treeCtrl->SelectItem(m_id);
}

bool TreeViewItemWx::isVisible() const
{
	return true;
}

void TreeViewItemWx::show()
{
}

void TreeViewItemWx::setEditable(bool editable)
{
}

bool TreeViewItemWx::isEditable() const
{
	return true;
}

bool TreeViewItemWx::edit()
{
	return m_treeCtrl->EditLabel(m_id) != 0;
}

void TreeViewItemWx::sort(bool recursive)
{
}

Ref< TreeViewItem > TreeViewItemWx::getParent() const
{
	return m_parent;
}

bool TreeViewItemWx::hasChildren() const
{
	return m_treeCtrl->GetChildrenCount(m_id, false) > 0;
}

int TreeViewItemWx::getChildren(RefArray< TreeViewItem >& outChildren) const
{
	wxTreeItemIdValue cookie;

	wxTreeItemId childId = m_treeCtrl->GetFirstChild(m_id, cookie);
	while (childId.IsOk())
	{
		Ref< TreeViewItemWx > childItem = static_cast< TreeItemData* >(m_treeCtrl->GetItemData(childId))->getItem();
		T_ASSERT (childItem);

		outChildren.push_back(childItem);

		childId = m_treeCtrl->GetNextChild(m_id, cookie);
	}

	return int(outChildren.size());
}

	}
}
