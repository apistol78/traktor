
#import <Cocoa/Cocoa.h>

@interface NSListDataSource : NSObject

- (int) numberOfRowsInTableView: (NSTableView*)tableView;

- (id) tableView: (NSTableView*)tableView objectValueForTableColumn: (NSTableColumn*)tableColumn row: (int)row;

@end
