#import "Ui/Cocoa/NSListDataSource.h"

@implementation NSListDataSource

- (int) numberOfRowsInTableView: (NSTableView*)tableView
{
	return 2;
}

- (id) tableView: (NSTableView*)tableView objectValueForTableColumn: (NSTableColumn*)tableColumn row: (int)row
{
	return @"Hello";
}

@end
