#import "Ui/Cocoa/NSTreeDataSource.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

using namespace traktor::ui;

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
	std::wstring value;
	bool bold;
	
	m_callback->treeValue(item, value, bold);
	NSString* s = makeNSString(value);

	if (bold)
	{
		CGFloat fontSize = [NSFont systemFontSize];
		NSFont* font = [NSFont boldSystemFontOfSize: fontSize];
	
		NSMutableDictionary* attributes = [NSMutableDictionary dictionary];
		[attributes setObject: font forKey:NSFontAttributeName];
	
		NSAttributedString* as = [[[NSAttributedString alloc] initWithString: s attributes: attributes] autorelease];
		return as;
	}
	else
		return s;
}

- (void) outlineView:(NSOutlineView*)outlineView setObjectValue:(id)object forTableColumn:(NSTableColumn*)tableColumn byItem:(id)item
{
	NSString* str = (NSString*)object;
	m_callback->treeSetValue(item, fromNSString(str));
}

@end
