#ifndef traktor_ui_TreeViewItemCocoa_H
#define traktor_ui_TreeViewItemCocoa_H

#include "Core/Heap/Ref.h"
#include "Ui/TreeViewItem.h"

namespace traktor
{
	namespace ui
	{
	
class TreeViewItemCocoa : public TreeViewItem
{
	T_RTTI_CLASS(TreeViewItemCocoa)
	
public:
	TreeViewItemCocoa();
	
	virtual ~TreeViewItemCocoa();
	
	virtual void setText(const std::wstring& text);
	
	virtual std::wstring getText() const;
	
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

	virtual bool edit();
	
	virtual TreeViewItem* getParent() const;

	virtual bool hasChildren() const;
	
	virtual int getChildren(RefArray< TreeViewItem >& outChildren) const;
};
	
	}
}

#endif	// traktor_ui_TreeViewItemCocoa_H
