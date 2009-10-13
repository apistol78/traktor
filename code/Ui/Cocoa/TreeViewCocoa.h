#ifndef traktor_ui_TreeViewCocoa_H
#define traktor_ui_TreeViewCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Cocoa/NSTreeDataSource.h"
#include "Ui/Itf/ITreeView.h"

namespace traktor
{
	namespace ui
	{
	
class TreeViewItemCocoa;

class TreeViewCocoa
:	public WidgetCocoaImpl< ITreeView, NSOutlineView >
,	public ITreeDataCallback
{
public:
	TreeViewCocoa(EventSubject* owner);
	
	// ITreeView

	virtual bool create(IWidget* parent, int style);

	virtual int addImage(IBitmap* image, int imageCount);

	virtual TreeViewItem* createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage);

	virtual void removeItem(TreeViewItem* item);

	virtual void removeAllItems();

	virtual TreeViewItem* getRootItem() const;

	virtual TreeViewItem* getSelectedItem() const;
	
	// ITreeDataCallback
	
	virtual void* treeChildOfItem(int childIndex, void* item) const;
	
	virtual bool treeIsExpandable(void* item) const;
	
	virtual int treeNumberOfChildren(void* item) const;
	
	virtual std::wstring treeValue(void* item) const;
	
private:
	Ref< TreeViewItemCocoa > m_rootItem;
	
	TreeViewItemCocoa* getRealItem(void* item) const;
};

	}
}

#endif	// traktor_ui_TreeViewCocoa_H
