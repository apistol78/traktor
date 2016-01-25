#ifndef traktor_ui_TreeViewCocoa_H
#define traktor_ui_TreeViewCocoa_H

#import "Ui/Cocoa/ObjCRef.h"
#import "Ui/Cocoa/NSCustomOutlineView.h"

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Cocoa/NSTreeDataSource.h"
#include "Ui/Cocoa/NSTargetProxy.h"
#include "Ui/Cocoa/NSOutlineViewDelegateProxy.h"
#include "Ui/Itf/ITreeView.h"

namespace traktor
{
	namespace ui
	{
	
class TreeViewItemCocoa;
class BitmapCocoa;

class TreeViewCocoa
:	public WidgetCocoaImpl< ITreeView, NSCustomOutlineView, NSScrollView >
,	public ITreeDataCallback
,	public ITargetProxyCallback
,	public INSOutlineViewEventsCallback
{
public:
	TreeViewCocoa(EventSubject* owner);
		
	// ITreeView

	virtual bool create(IWidget* parent, int style);

	virtual int addImage(ISystemBitmap* image, int imageCount);

	virtual Ref< TreeViewItem > createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage);

	virtual void removeItem(TreeViewItem* item);

	virtual void removeAllItems();

	virtual Ref< TreeViewItem > getRootItem() const;

	virtual Ref< TreeViewItem > getSelectedItem() const;
	
	// ITreeDataCallback
	
	virtual void* treeChildOfItem(int childIndex, void* item) const;
	
	virtual bool treeIsExpandable(void* item) const;
	
	virtual int treeNumberOfChildren(void* item) const;
	
	virtual void treeValue(void* item, std::wstring& outValue, bool& outBold) const;
	
	virtual void treeSetValue(void* item, const std::wstring& value);
	
	// ITargetProxyCallback
	
	virtual void targetProxy_Action(void* controlId);
	
	virtual void targetProxy_doubleAction(void* controlId);
	
	// INSOutlineViewEventsCallback
	
	virtual void event_selectionDidChange();
	
	virtual void event_rightMouseDown(NSEvent* event);
	
	virtual void event_willDisplayCell(NSCell* cell, NSTableColumn* tableColumn, void* item);

private:
	Ref< TreeViewItemCocoa > m_rootItem;
	ObjCRef* m_rootItemRef;
	std::vector< BitmapCocoa* > m_bitmaps;
	
	TreeViewItemCocoa* getRealItem(void* item) const;
};

	}
}

#endif	// traktor_ui_TreeViewCocoa_H
