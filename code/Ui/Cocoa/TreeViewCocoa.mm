#include "Ui/Cocoa/TreeViewCocoa.h"
#include "Ui/Cocoa/TreeViewItemCocoa.h"
#include "Ui/Cocoa/ObjCRef.h"
#include "Core/Heap/GcNew.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{
	
TreeViewCocoa::TreeViewCocoa(EventSubject* owner)
:	WidgetCocoaImpl< ITreeView, NSOutlineView >(owner)
{
}

bool TreeViewCocoa::create(IWidget* parent, int style)
{
	NSTreeDataSource* dataSource = [[[NSTreeDataSource alloc] init] autorelease];
	[dataSource setCallback: this];

	NSTableColumn* column = [[NSTableColumn alloc] initWithIdentifier: nil];

	m_control = [[NSOutlineView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setColumnAutoresizingStyle: NSTableViewUniformColumnAutoresizingStyle];
	[m_control addTableColumn: column];
	[m_control setDataSource: dataSource];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];

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

	if (!realParent)
		m_rootItem = item;
	else
		realParent->addChild(item);

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
			
		return [[ObjCRef alloc] initWithRef: m_rootItem];
	}

	Ref< TreeViewItemCocoa > realItem = getRealItem(item);
	T_ASSERT (realItem);
	
	const RefArray< TreeViewItemCocoa >& realChildItems = realItem->getChildren();
	if (childIndex >= int(realChildItems.size()))
		return 0;
		
	return [[ObjCRef alloc] initWithRef: realChildItems[childIndex]];
}

bool TreeViewCocoa::treeIsExpandable(void* item) const
{
	Ref< TreeViewItemCocoa > realItem = getRealItem(item);
	T_ASSERT (realItem);

	const RefArray< TreeViewItemCocoa >& realChildItems = realItem->getChildren();
	return !realChildItems.empty();
}

int TreeViewCocoa::treeNumberOfChildren(void* item) const
{
	if (!item)
		return m_rootItem ? 1 : 0;

	Ref< TreeViewItemCocoa > realItem = getRealItem(item);
	T_ASSERT (item);

	const RefArray< TreeViewItemCocoa >& realChildItems = realItem->getChildren();
	return int(realChildItems.size());
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
