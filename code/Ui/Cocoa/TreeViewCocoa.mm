#include "Ui/Cocoa/TreeViewCocoa.h"
#include "Ui/Cocoa/TreeViewItemCocoa.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/EventSubject.h"
#include "Core/Heap/GcNew.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{
	
TreeViewCocoa::TreeViewCocoa(EventSubject* owner)
:	WidgetCocoaImpl< ITreeView, NSOutlineView, NSScrollView >(owner)
,	m_rootItemRef(0)
{
}

bool TreeViewCocoa::create(IWidget* parent, int style)
{
	NSTargetProxy* targetProxy = [[NSTargetProxy alloc] init];
	[targetProxy setCallback: this];

	m_view = [[[NSScrollView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)] autorelease];
	[m_view setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
	[m_view setHasVerticalScroller: YES];
	[m_view setHasHorizontalScroller: YES];

	NSTreeDataSource* dataSource = [[[NSTreeDataSource alloc] init] autorelease];
	[dataSource setCallback: this];

	NSOutlineViewDelegateProxy* delegateProxy = [[[NSOutlineViewDelegateProxy alloc] init] autorelease];
	[delegateProxy setCallback: this];

	NSTableColumn* column = [[NSTableColumn alloc] initWithIdentifier: nil];
	[column setEditable: NO];
	
	NSCell* dataCell = [column dataCell];
	[dataCell setFont: [NSFont controlContentFontOfSize: 11]];

	m_control = [[NSOutlineView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setAutoresizesOutlineColumn: NO];
	[m_control addTableColumn: column];
	[m_control setOutlineTableColumn: column];
	[m_control setHeaderView: nil];
	[m_control setDataSource: dataSource];
	[m_control setTarget: targetProxy];
	[m_control setAction: @selector(dispatchActionCallback:)];
	[m_control setDoubleAction: @selector(dispatchDoubleActionCallback:)];
	[m_control setDelegate: delegateProxy];
	
	[m_view setDocumentView: m_control];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_view];
	
	return true;
}

int TreeViewCocoa::addImage(IBitmap* image, int imageCount)
{
	return 0;
}

TreeViewItem* TreeViewCocoa::createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage)
{
	Ref< TreeViewItemCocoa > realParent = checked_type_cast< TreeViewItemCocoa* >(parent);

	Ref< TreeViewItemCocoa > item = gc_new< TreeViewItemCocoa >(realParent);
	item->setText(text);
	item->setImage(image);
	item->setExpandedImage(expandedImage);
	
	ObjCRef* ref = [[ObjCRef alloc] initWithRef: item];

	if (!realParent)
	{
		m_rootItem = item;
		m_rootItemRef = ref;
	}
	else
		realParent->addChild(ref);

	[m_control reloadData];
	return item;
}

void TreeViewCocoa::removeItem(TreeViewItem* item)
{
	[m_control reloadData];
}

void TreeViewCocoa::removeAllItems()
{
	[m_control reloadData];
}

TreeViewItem* TreeViewCocoa::getRootItem() const
{
	return m_rootItem;
}

TreeViewItem* TreeViewCocoa::getSelectedItem() const
{
	int row = [m_control selectedRow];
	
	void* item = [m_control itemAtRow: row];
	if (!item)
		return 0;
	
	return getRealItem(item);
}

void* TreeViewCocoa::treeChildOfItem(int childIndex, void* item) const
{
	if (!item)
	{
		if (childIndex != 0)
			return 0;
			
		return m_rootItemRef;
	}

	Ref< TreeViewItemCocoa > realItem = getRealItem(item);
	T_ASSERT (realItem);
	
	const std::vector< ObjCRef* >& childItems = realItem->getChildren();
	if (childIndex >= int(childItems.size()))
		return 0;
		
	return childItems[childIndex];
}

bool TreeViewCocoa::treeIsExpandable(void* item) const
{
	Ref< TreeViewItemCocoa > realItem = getRealItem(item);
	T_ASSERT (realItem);
	
	return realItem->hasChildren();
}

int TreeViewCocoa::treeNumberOfChildren(void* item) const
{
	if (!item)
		return m_rootItem ? 1 : 0;

	Ref< TreeViewItemCocoa > realItem = getRealItem(item);
	T_ASSERT (item);

	const std::vector< ObjCRef* >& childItems = realItem->getChildren();
	return int(childItems.size());
}

std::wstring TreeViewCocoa::treeValue(void* item) const
{
	Ref< TreeViewItemCocoa > realItem = getRealItem(item);
	T_ASSERT (realItem);

	return realItem->getText();
}

void TreeViewCocoa::targetProxy_Action(void* controlId)
{
}
	
void TreeViewCocoa::targetProxy_doubleAction(void* controlId)
{
	int row = [m_control clickedRow];
	
	void* item = [m_control itemAtRow: row];
	T_ASSERT (item);
	
	Ref< TreeViewItemCocoa > realItem = getRealItem(item);
	T_ASSERT (realItem);
	
	CommandEvent commandEvent(m_owner, realItem);
	m_owner->raiseEvent(EiActivate, &commandEvent);
}

void TreeViewCocoa::event_selectionDidChange()
{	
	CommandEvent commandEvent(m_owner, getSelectedItem());
	m_owner->raiseEvent(EiSelectionChange, &commandEvent);
}

TreeViewItemCocoa* TreeViewCocoa::getRealItem(void* item) const
{
	if (item)
		return dynamic_type_cast< TreeViewItemCocoa* >([(ObjCRef*)item get]);
	else
		return m_rootItem;
}

	}
}
