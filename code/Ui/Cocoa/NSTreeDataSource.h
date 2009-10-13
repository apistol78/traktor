#ifndef traktor_ui_NSTreeDataSource_H
#define traktor_ui_NSTreeDataSource_H

#include <string>
#import <Cocoa/Cocoa.h>

struct ITreeDataCallback
{
	virtual void* treeChildOfItem(int childIndex, void* item) const = 0;
	
	virtual bool treeIsExpandable(void* item) const = 0;
	
	virtual int treeNumberOfChildren(void* item) const = 0;
	
	virtual std::wstring treeValue(void* item) const = 0;
};

@interface NSTreeDataSource : NSObject
{
	ITreeDataCallback* m_callback;
}

- (void) setCallback: (ITreeDataCallback*)callback;

- (id) outlineView:(NSOutlineView*)outlineView child:(int)index ofItem:(id)item;

- (BOOL) outlineView:(NSOutlineView*)outlineView isItemExpandable:(id)item;

- (int) outlineView:(NSOutlineView*)outlineView numberOfChildrenOfItem:(id)item;

- (id) outlineView:(NSOutlineView*)outlineView objectValueForTableColumn:(NSTableColumn*)tableColumn byItem:(id)item;

- (BOOL) outlineView:(NSOutlineView*)outlineView shouldEditTableColumn:(NSTableColumn*)tableColumn item:(id)item;

@end

#endif	// traktor_ui_NSTreeDataSource_H
