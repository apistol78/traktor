#import "Ui/Cocoa/NSTreeDataSource.h"

@implementation NSTreeDataSource

- (id) outlineView:(NSOutlineView*)outlineView child:(NSInteger)index ofItem:(id)item
{
	return nil;
}

- (BOOL) outlineView:(NSOutlineView*)outlineView isItemExpandable:(id)item
{
	return NO;
}

- (NSInteger) outlineView:(NSOutlineView*)outlineView numberOfChildrenOItem:(id)item
{
	return 0;
}

- (id) outlineView:(NSOutlineView*)outlineView objectValueForTableColumn:(NSTableColumn*)tableColumn byItem:(id)item
{
	return nil;
}

@end
