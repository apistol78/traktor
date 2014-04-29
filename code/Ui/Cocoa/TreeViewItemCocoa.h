#ifndef traktor_ui_TreeViewItemCocoa_H
#define traktor_ui_TreeViewItemCocoa_H

#import "Ui/Cocoa/ObjCRef.h"

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
