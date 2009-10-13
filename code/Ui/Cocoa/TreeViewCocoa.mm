#include "Ui/Cocoa/TreeViewCocoa.h"
#include "Ui/Cocoa/TreeViewItemCocoa.h"
#include "Core/Heap/GcNew.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{
	
TreeViewCocoa::TreeViewCocoa(EventSubject* owner)
:	WidgetCocoaImpl< ITreeView, NSOutlineView >(owner)
,	m_rootItemRef(0)
{
}

bool TreeViewCocoa::create(IWidget* parent, int style)
{
	NSScrollView* scrollView = [[[NSScrollView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)] autorelease];
	[scrollView setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
	[scrollView setHasVerticalScroller: YES];
	[scrollView setHasHorizontalScroller: YES];

	NSTreeDataSource* dataSource = [[[NSTreeDataSource alloc] init] autorelease];
	[dataSource setCallback: this];

	NSTableColumn* column = [[NSTableColumn alloc] initWithIdentifier: nil];
	[column setEditable: NO];

	m_control = [[NSOutlineView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setAutoresizesOutlineColumn: NO];
	[m_control addTableColumn: column];
	[m_control setOutlineTableColumn: column];
	[m_control setDataSource: dataSource];
	
	[scrollView setDocumentView: m_control];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: scrollView];
	
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
	return 0;
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

TreeViewItemCocoa* TreeViewCocoa::getRealItem(void* item) const
{
	if (item)
		return dynamic_type_cast< TreeViewItemCocoa* >([(ObjCRef*)item get]);
	else
		return m_rootItem;
}

	}
}
