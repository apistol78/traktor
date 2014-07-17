#include "Ui/ListViewItem.h"
#include "Ui/ListViewItems.h"
#include "Ui/Cocoa/ListViewCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

namespace traktor
{
	namespace ui
	{

ListViewCocoa::ListViewCocoa(EventSubject* owner)
:	WidgetCocoaImpl< IListView, NSTableView, NSScrollView >(owner)
{
}

bool ListViewCocoa::create(IWidget* parent, int style)
{
	m_view = [[NSScrollView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_view setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
	[m_view setHasVerticalScroller: YES];
	[m_view setHasHorizontalScroller: YES];

	NSListDataSource* dataSource = [[NSListDataSource alloc] init];
	[dataSource setCallback: this];

	m_control = [[NSTableView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setColumnAutoresizingStyle: NSTableViewUniformColumnAutoresizingStyle];
	[m_control setDataSource: dataSource];
	
	[m_view setDocumentView: m_control];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_view];
	
	return true;
}

void ListViewCocoa::setStyle(int style)
{
}

int ListViewCocoa::addImage(IBitmap* image, int imageCount, bool smallImage)
{
	return 0;
}

void ListViewCocoa::removeAllColumns()
{
}

void ListViewCocoa::removeColumn(int columnIndex)
{
	NSArray* tableColumns = [m_control tableColumns];
	NSTableColumn* column = (NSTableColumn*)[tableColumns objectAtIndex: columnIndex];
	[m_control removeTableColumn: column];
}

int ListViewCocoa::getColumnCount() const
{
	return [m_control numberOfColumns];
}

int ListViewCocoa::addColumn(const std::wstring& columnHeader, int width)
{
	NSTableColumn* column = [[NSTableColumn alloc] initWithIdentifier: nil];
	[column setEditable: NO];
	[column setWidth: width];
	
	NSCell* headerCell = [column headerCell];
	[headerCell setStringValue: makeNSString(columnHeader)];
	
	NSCell* dataCell = [column dataCell];
	[dataCell setFont: [NSFont controlContentFontOfSize: 11]];
	
	[m_control addTableColumn: column];
}

void ListViewCocoa::setColumnHeader(int columnIndex, const std::wstring& columnHeader)
{
	NSArray* tableColumns = [m_control tableColumns];
	NSTableColumn* column = (NSTableColumn*)[tableColumns objectAtIndex: columnIndex];
	
	NSCell* headerCell = [column headerCell];
	[headerCell setStringValue: makeNSString(columnHeader)];
}

std::wstring ListViewCocoa::getColumnHeader(int columnIndex) const
{
	NSArray* tableColumns = [m_control tableColumns];
	NSTableColumn* column = (NSTableColumn*)[tableColumns objectAtIndex: columnIndex];
	
	NSCell* headerCell = [column headerCell];
	return fromNSString([headerCell stringValue]);
}

void ListViewCocoa::setColumnWidth(int columnIndex, int width)
{
	NSArray* tableColumns = [m_control tableColumns];
	NSTableColumn* column = (NSTableColumn*)[tableColumns objectAtIndex: columnIndex];
	[column setWidth: width];
}

int ListViewCocoa::getColumnWidth(int columnIndex) const
{
	NSArray* tableColumns = [m_control tableColumns];
	NSTableColumn* column = (NSTableColumn*)[tableColumns objectAtIndex: columnIndex];
	return [column width];
}

void ListViewCocoa::setItems(ListViewItems* items)
{
	m_items = items;
	[m_control reloadData];
}

Ref< ListViewItems > ListViewCocoa::getItems() const
{
	return m_items;
}

int ListViewCocoa::getSelectedItems(std::vector< int >& items) const
{
	NSIndexSet* selectedRowIndices = [m_control selectedRowIndexes];
	if (!selectedRowIndices)
		return 0;
		
	NSUInteger first = [selectedRowIndices firstIndex];
	if (first == NSNotFound)
		return 0;

	NSUInteger last = [selectedRowIndices lastIndex];
	
	for (NSUInteger index = first; index <= last; ++index)
	{
		if ([selectedRowIndices containsIndex: index])
			items.push_back(int(index));
	}
	
	return int(items.size());
}

int ListViewCocoa::listCount() const
{
	return m_items ? m_items->count() : 0;
}

std::wstring ListViewCocoa::listValue(NSTableColumn* tableColumn, int index) const
{
	if (!m_items || index >= m_items->count())
		return L"";

	NSArray* tableColumns = [m_control tableColumns];
	NSUInteger columnIndex = [tableColumns indexOfObject: tableColumn];
	if (columnIndex == NSNotFound)
		return L"";
				
	Ref< ListViewItem > item = m_items->get(index);
	if (!item)
		return L"";

	return item->getText(columnIndex);
}

	}
}
