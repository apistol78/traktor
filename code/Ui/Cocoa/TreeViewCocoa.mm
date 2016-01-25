#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Ui/EventSubject.h"
#include "Ui/TreeView.h"
#include "Ui/Events/AllEvents.h"
#include "Ui/Cocoa/TreeViewCocoa.h"
#include "Ui/Cocoa/TreeViewItemCocoa.h"
#include "Ui/Cocoa/BitmapCocoa.h"

namespace traktor
{
	namespace ui
	{
	
TreeViewCocoa::TreeViewCocoa(EventSubject* owner)
:	WidgetCocoaImpl< ITreeView, NSCustomOutlineView, NSScrollView >(owner)
,	m_rootItemRef(0)
{
}

bool TreeViewCocoa::create(IWidget* parent, int style)
{
	NSTargetProxy* targetProxy = [[NSTargetProxy alloc] init];
	[targetProxy setCallback: this];

	m_view = [[NSScrollView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_view setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
	[m_view setHasVerticalScroller: YES];
	[m_view setHasHorizontalScroller: YES];

	NSTreeDataSource* dataSource = [[NSTreeDataSource alloc] init];
	[dataSource setCallback: this];

	NSOutlineViewDelegateProxy* delegateProxy = [[NSOutlineViewDelegateProxy alloc] init];
	[delegateProxy setCallback: this];

	NSTableColumn* column = [[NSTableColumn alloc] initWithIdentifier: nil];
	if (style & TreeView::WsAutoEdit)
		[column setEditable: YES];
	else
		[column setEditable: NO];
	
	NSBrowserCell* cell = [[NSBrowserCell alloc] init];
	[cell setLeaf: YES];
	[column setDataCell: cell];

	m_control = [[NSCustomOutlineView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
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
	
	return class_t::create();
}

int TreeViewCocoa::addImage(ISystemBitmap* image, int imageCount)
{
	Ref< drawing::Image > images = image->getImage()->clone();
	if (!images)
		return 0;
		
	drawing::MirrorFilter mirrorFilter(false, true);
	images->apply(&mirrorFilter);
	
	int32_t width = images->getWidth() / imageCount;
	int32_t height = images->getHeight();
	
	int32_t base = int32_t(m_bitmaps.size());
	
	for (int32_t i = 0; i < imageCount; ++i)
	{
		BitmapCocoa* bm = new BitmapCocoa();
		bm->create(width, height);
		bm->copySubImage(
			images,
			Rect(Point(i * width, 0), Size(width, height)),
			Point(0, 0)
		);
		m_bitmaps.push_back(bm);
	}

	return base;
}

Ref< TreeViewItem > TreeViewCocoa::createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage)
{
	Ref< TreeViewItemCocoa > realParent = checked_type_cast< TreeViewItemCocoa* >(parent);

	Ref< TreeViewItemCocoa > item = new TreeViewItemCocoa(realParent);
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

Ref< TreeViewItem > TreeViewCocoa::getRootItem() const
{
	return m_rootItem;
}

Ref< TreeViewItem > TreeViewCocoa::getSelectedItem() const
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

void TreeViewCocoa::treeValue(void* item, std::wstring& outValue, bool& outBold) const
{
	Ref< TreeViewItemCocoa > realItem = getRealItem(item);
	T_ASSERT (realItem);

	outValue = realItem->getText();
	outBold = realItem->isBold();
}

void TreeViewCocoa::treeSetValue(void* item, const std::wstring& value)
{
	Ref< TreeViewItemCocoa > realItem = getRealItem(item);
	T_ASSERT (realItem);
	
	realItem->setText(value);
	
	TreeViewContentChangeEvent contentChangeEvent(m_owner, realItem);
	m_owner->raiseEvent(&contentChangeEvent);
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
	
	TreeViewItemActivateEvent activateEvent(m_owner, realItem);
	m_owner->raiseEvent(&activateEvent);
}

void TreeViewCocoa::event_selectionDidChange()
{	
	SelectionChangeEvent selectionChangeEvent(m_owner, getSelectedItem());
	m_owner->raiseEvent(&selectionChangeEvent);
}

void TreeViewCocoa::event_rightMouseDown(NSEvent* event)
{
	NSPoint mousePosition = [event locationInWindow];
	mousePosition = [m_control convertPointFromBase: mousePosition];

	MouseButtonDownEvent mouseEvent(
		m_owner,
		MbtRight,
		fromNSPoint(mousePosition)
	);
	m_owner->raiseEvent(&mouseEvent);
}

void TreeViewCocoa::event_willDisplayCell(NSCell* cell, NSTableColumn* tableColumn, void* item)
{
	Ref< TreeViewItemCocoa > realItem = getRealItem(item);
	T_ASSERT (realItem);
	
	if ([m_control outlineTableColumn] != tableColumn)
		return;
	
	int32_t index = realItem->getImage();
	if (index >= 0 && index < int32_t(m_bitmaps.size()))
	{
		BitmapCocoa* bitmap = m_bitmaps[index];
		if (bitmap)
			[cell setImage: bitmap->getNSImage()];
		else
			[cell setImage: nil];
	}
	else
		[cell setImage: nil];
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
