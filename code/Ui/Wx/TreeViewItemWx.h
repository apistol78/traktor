#ifndef traktor_ui_TreeViewItemWx_H
#define traktor_ui_TreeViewItemWx_H

#include <wx/treectrl.h>
#include "Core/Heap/Ref.h"
#include "Ui/TreeViewItem.h"

namespace traktor
{
	namespace ui
	{
	
class TreeViewItemWx : public TreeViewItem
{
	T_RTTI_CLASS(TreeViewItemWx)
	
public:
	TreeViewItemWx(wxTreeCtrl* treeCtrl, TreeViewItem* parent, wxTreeItemId id);
	
	virtual ~TreeViewItemWx();
	
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

	const wxTreeItemId& getId() const { return m_id; }

private:
	wxTreeCtrl* m_treeCtrl;
	Ref< TreeViewItem > m_parent;
	wxTreeItemId m_id;
};
	
	}
}

#endif	// traktor_ui_TreeViewItemWx_H
