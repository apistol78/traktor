#import "Ui/Cocoa/NSTreeDataSource.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

@implementation NSTreeDataSource

- (void) setCallback: (ITreeDataCallback*)callback
{
	m_callback = callback;
}

- (id) outlineView:(NSOutlineView*)outlineView child:(int)index ofItem:(id)item
{
	return (id)m_callback->treeChildOfItem(index, item);
}

- (BOOL) outlineView:(NSOutlineView*)outlineView isItemExpandable:(id)item
{
	return m_callback->treeIsExpandable(item) ? YES : NO;
}

- (int) outlineView:(NSOutlineView*)outlineView numberOfChildrenOfItem:(id)item
{
	return m_callback->treeNumberOfChildren(item);
}

- (id) outlineView:(NSOutlineView*)outlineView objectValueForTableColumn:(NSTableColumn*)tableColumn byItem:(id)item
{
	return traktor::ui::makeNSString(m_callback->treeValue(item));
}

- (BOOL) outlineView:(NSOutlineView*)outlineView shouldEditTableColumn:(NSTableColumn*)tableColumn item:(id)item
{
	return NO;
}

@end
