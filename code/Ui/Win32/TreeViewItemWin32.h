#ifndef traktor_ui_TreeViewItemWin32_H
#define traktor_ui_TreeViewItemWin32_H

#include "Ui/TreeViewItem.h"
#include "Ui/Win32/Window.h"

namespace traktor
{
	namespace ui
	{

class TreeViewWin32;

/*! \brief
 * \ingroup UIW32
 */
class TreeViewItemWin32 : public TreeViewItem
{
	T_RTTI_CLASS;

public:
	TreeViewItemWin32(TreeViewWin32* owner, TreeViewItem* parent, int image, int expandedImage, HWND hWndTree);

	virtual ~TreeViewItemWin32();

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

	TreeViewWin32* getOwner() const;

	void setHandle(HTREEITEM hItem);

	HTREEITEM getHandle() const;

	void updateImage();

private:
	TreeViewWin32* m_owner;
	int m_image;
	int m_expandedImage;
	HWND m_hWndTree;
	HTREEITEM m_hItem;
	bool m_editable;

	static int CALLBACK compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};

	}
}

#endif	// traktor_ui_TreeViewItemWin32_H
