#import "Ui/Cocoa/NSListDataSource.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

@implementation NSListDataSource

- (void) setCallback: (IListDataCallback*)callback
{
	m_callback = callback;
}

- (int) numberOfRowsInTableView: (NSTableView*)tableView
{
	return m_callback->listCount();
}

- (id) tableView: (NSTableView*)tableView objectValueForTableColumn: (NSTableColumn*)tableColumn row: (int)row
{
	std::wstring str = m_callback->listValue(tableColumn, row);
	return traktor::ui::makeNSString(str);
}

@end
