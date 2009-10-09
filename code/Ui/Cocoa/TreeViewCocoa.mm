#include "Ui/Cocoa/TreeViewCocoa.h"
#include "Ui/Cocoa/TreeViewItemCocoa.h"
#include "Core/Heap/GcNew.h"

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
	m_control = [[NSOutlineView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	
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
	return gc_new< TreeViewItemCocoa >();
}

void TreeViewCocoa::removeItem(TreeViewItem* item)
{
}

void TreeViewCocoa::removeAllItems()
{
}

TreeViewItem* TreeViewCocoa::getRootItem() const
{
	return 0;
}

TreeViewItem* TreeViewCocoa::getSelectedItem() const
{
	return 0;
}
	
	}
}
