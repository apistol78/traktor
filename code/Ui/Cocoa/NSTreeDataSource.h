/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_NSTreeDataSource_H
#define traktor_ui_NSTreeDataSource_H

#include <string>
#import <Cocoa/Cocoa.h>

struct ITreeDataCallback
{
	virtual void* treeChildOfItem(int childIndex, void* item) const = 0;
	
	virtual bool treeIsExpandable(void* item) const = 0;
	
	virtual int treeNumberOfChildren(void* item) const = 0;
	
	virtual void treeValue(void* item, std::wstring& outValue, bool& outBold) const = 0;
	
	virtual void treeSetValue(void* item, const std::wstring& value) = 0;
};

@interface NSTreeDataSource : NSObject < NSOutlineViewDataSource >
{
	ITreeDataCallback* m_callback;
}

- (void) setCallback: (ITreeDataCallback*)callback;

- (id) outlineView:(NSOutlineView*)outlineView child:(int)index ofItem:(id)item;

- (BOOL) outlineView:(NSOutlineView*)outlineView isItemExpandable:(id)item;

- (int) outlineView:(NSOutlineView*)outlineView numberOfChildrenOfItem:(id)item;

- (id) outlineView:(NSOutlineView*)outlineView objectValueForTableColumn:(NSTableColumn*)tableColumn byItem:(id)item;

- (void) outlineView:(NSOutlineView*)outlineView setObjectValue:(id)object forTableColumn:(NSTableColumn*)tableColumn byItem:(id)item;

@end

#endif	// traktor_ui_NSTreeDataSource_H
