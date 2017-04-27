/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/

#import <Cocoa/Cocoa.h>

#include <string>

struct IListDataCallback
{
	virtual int listCount() const = 0;
	
	virtual std::wstring listValue(NSTableColumn* tableColumn, int index) const = 0;
};

@interface NSListDataSource : NSObject < NSTableViewDataSource >
{
	IListDataCallback* m_callback;
}

- (id) init;

- (void) setCallback: (IListDataCallback*)callback;

- (int) numberOfRowsInTableView: (NSTableView*)tableView;

- (id) tableView: (NSTableView*)tableView objectValueForTableColumn: (NSTableColumn*)tableColumn row: (int)row;

@end
