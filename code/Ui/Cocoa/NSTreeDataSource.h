#ifndef traktor_ui_NSTreeDataSource_H
#define traktor_ui_NSTreeDataSource_H

#import <Cocoa/Cocoa.h>

@interface NSTreeDataSource : NSObject

- (id) outlineView:(NSOutlineView*)outlineView child:(NSInteger)index ofItem:(id)item;

- (BOOL) outlineView:(NSOutlineView*)outlineView isItemExpandable:(id)item;

- (NSInteger) outlineView:(NSOutlineView*)outlineView numberOfChildrenOItem:(id)item;

- (id) outlineView:(NSOutlineView*)outlineView objectValueForTableColumn:(NSTableColumn*)tableColumn byItem:(id)item;

@end

#endif	// traktor_ui_NSTreeDataSource_H
