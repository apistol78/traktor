/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_TreeViewItemWx_H
#define traktor_ui_TreeViewItemWx_H

#include <wx/treectrl.h>
#include "Ui/TreeViewItem.h"

namespace traktor
{
	namespace ui
	{
	
class TreeViewItemWx : public TreeViewItem
{
	T_RTTI_CLASS;
	
public:
	TreeViewItemWx(wxTreeCtrl* treeCtrl, TreeViewItem* parent, wxTreeItemId id);
	
	virtual ~TreeViewItemWx();
	
	virtual void setText(const std::wstring& text);
	
	virtual std::wstring getText() const;
	
	virtual void setBold(bool bold);

	virtual bool isBold() const;

	virtual void setImage(int image);
	
	virtual int getImage() const;
	
	virtual void setExpandedImage(int expandedImage);
	
	virtual int getExpandedImage() const;
	
	virtual bool isExpanded() const;
	
	virtual void expand();
	
	virtual bool isCollapsed() const;
	
	virtual void collapse();
	
	virtual bool isSelected() const;
	
	virtual void select();

	virtual bool isVisible() const;

	virtual void show();

	virtual void setEditable(bool editable);

	virtual bool isEditable() const;

	virtual bool edit();

	virtual void sort(bool recursive);
	
	virtual Ref< TreeViewItem > getParent() const;

	virtual bool hasChildren() const;
	
	virtual int getChildren(RefArray< TreeViewItem >& outChildren) const;

	const wxTreeItemId& getId() const { return m_id; }

private:
	wxTreeCtrl* m_treeCtrl;
	Ref< TreeViewItem > m_parent;
	wxTreeItemId m_id;
};
	
	}
}

#endif	// traktor_ui_TreeViewItemWx_H
