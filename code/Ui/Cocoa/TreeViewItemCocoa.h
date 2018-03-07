/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_TreeViewItemCocoa_H
#define traktor_ui_TreeViewItemCocoa_H

#import "Ui/Cocoa/ObjCRef.h"

#include <vector>
#include "Core/Ref.h"
#include "Ui/TreeViewItem.h"

namespace traktor
{
	namespace ui
	{
	
class TreeViewItemCocoa : public TreeViewItem
{
	T_RTTI_CLASS;
	
public:
	TreeViewItemCocoa(TreeViewItemCocoa* parent);
	
	virtual ~TreeViewItemCocoa();
	
	virtual void setText(const std::wstring& text) T_OVERRIDE T_FINAL;
	
	virtual std::wstring getText() const T_OVERRIDE T_FINAL;
	
	virtual void setBold(bool bold) T_OVERRIDE T_FINAL;
	
	virtual bool isBold() const T_OVERRIDE T_FINAL;
	
	virtual void setImage(int image) T_OVERRIDE T_FINAL;
	
	virtual int getImage() const T_OVERRIDE T_FINAL;
	
	virtual void setExpandedImage(int expandedImage) T_OVERRIDE T_FINAL;
	
	virtual int getExpandedImage() const T_OVERRIDE T_FINAL;
	
	virtual bool isExpanded() const T_OVERRIDE T_FINAL;
	
	virtual void expand() T_OVERRIDE T_FINAL;
	
	virtual bool isCollapsed() const T_OVERRIDE T_FINAL;
	
	virtual void collapse() T_OVERRIDE T_FINAL;
	
	virtual bool isSelected() const T_OVERRIDE T_FINAL;
	
	virtual void select() T_OVERRIDE T_FINAL;
	
	virtual bool isVisible() const T_OVERRIDE T_FINAL;
	
	virtual void show() T_OVERRIDE T_FINAL;
	
	virtual void setEditable(bool editable) T_OVERRIDE T_FINAL;

	virtual bool isEditable() const T_OVERRIDE T_FINAL;

	virtual bool edit() T_OVERRIDE T_FINAL;

	virtual void sort(bool recursive) T_OVERRIDE T_FINAL;
	
	virtual Ref< TreeViewItem > getParent() const T_OVERRIDE T_FINAL;

	virtual bool hasChildren() const T_OVERRIDE T_FINAL;
	
	virtual int getChildren(RefArray< TreeViewItem >& outChildren) const T_OVERRIDE T_FINAL;
	
	const std::vector< ObjCRef* >& getChildren() const { return m_children; }

	void addChild(ObjCRef* childItem) { m_children.push_back(childItem); }
	
private:
	std::wstring m_text;
	bool m_bold;
	int m_image;
	int m_expandedImage;
	bool m_expanded;
	TreeViewItemCocoa* m_parent;
	std::vector< ObjCRef* > m_children;
};
	
	}
}

#endif	// traktor_ui_TreeViewItemCocoa_H
