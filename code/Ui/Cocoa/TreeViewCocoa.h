/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual bool create(IWidget* parent, int style) T_OVERRIDE T_FINAL;

	virtual int addImage(ISystemBitmap* image, int imageCount) T_OVERRIDE T_FINAL;

	virtual Ref< TreeViewItem > createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage) T_OVERRIDE T_FINAL;

	virtual void removeItem(TreeViewItem* item) T_OVERRIDE T_FINAL;

	virtual void removeAllItems() T_OVERRIDE T_FINAL;

	virtual Ref< TreeViewItem > getRootItem() const T_OVERRIDE T_FINAL;

	virtual Ref< TreeViewItem > getSelectedItem() const T_OVERRIDE T_FINAL;
	
	// ITreeDataCallback
	
	virtual void* treeChildOfItem(int childIndex, void* item) const T_OVERRIDE T_FINAL;
	
	virtual bool treeIsExpandable(void* item) const T_OVERRIDE T_FINAL;
	
	virtual int treeNumberOfChildren(void* item) const T_OVERRIDE T_FINAL;
	
	virtual void treeValue(void* item, std::wstring& outValue, bool& outBold) const T_OVERRIDE T_FINAL;
	
	virtual void treeSetValue(void* item, const std::wstring& value) T_OVERRIDE T_FINAL;
	
	// ITargetProxyCallback
	
	virtual void targetProxy_Action(void* controlId) T_OVERRIDE T_FINAL;
	
	virtual void targetProxy_doubleAction(void* controlId) T_OVERRIDE T_FINAL;
	
	// INSOutlineViewEventsCallback
	
	virtual void event_selectionDidChange() T_OVERRIDE T_FINAL;
	
	virtual void event_rightMouseDown(NSEvent* event) T_OVERRIDE T_FINAL;
	
	virtual void event_willDisplayCell(NSCell* cell, NSTableColumn* tableColumn, void* item) T_OVERRIDE T_FINAL;

private:
	Ref< TreeViewItemCocoa > m_rootItem;
	ObjCRef* m_rootItemRef;
	std::vector< BitmapCocoa* > m_bitmaps;
	
	TreeViewItemCocoa* getRealItem(void* item) const;
};

	}
}

#endif	// traktor_ui_TreeViewCocoa_H
