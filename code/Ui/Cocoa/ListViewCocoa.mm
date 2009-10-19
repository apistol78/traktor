#include "Ui/Cocoa/ListViewCocoa.h"

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
	m_view = [[[NSScrollView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)] autorelease];
	[m_view setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
	[m_view setHasVerticalScroller: YES];
	[m_view setHasHorizontalScroller: YES];

	m_control = [[NSTableView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setColumnAutoresizingStyle: NSTableViewUniformColumnAutoresizingStyle];
	
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
}

int ListViewCocoa::getColumnCount() const
{
	return [m_control numberOfColumns];
}

int ListViewCocoa::addColumn(const std::wstring& columnHeader, int width)
{
	NSTableColumn* column = [[NSTableColumn alloc] initWithIdentifier: makeNSString(columnHeader)];
	[column setEditable: NO];
	[column setWidth: width];
	
	NSCell* dataCell = [column dataCell];
	[dataCell setFont: [NSFont controlContentFontOfSize: 11]];
	
	[m_control addTableColumn: column];
}

void ListViewCocoa::setColumnHeader(int columnIndex, const std::wstring& columnHeader)
{
}

std::wstring ListViewCocoa::getColumnHeader(int columnIndex) const
{
	return L"";
}

void ListViewCocoa::setColumnWidth(int columnIndex, int width)
{
}

int ListViewCocoa::getColumnWidth(int columnIndex) const
{
	return 0;
}

void ListViewCocoa::setItems(ListViewItems* items)
{
}

ListViewItems* ListViewCocoa::getItems() const
{
	return 0;
}

int ListViewCocoa::getSelectedItems(std::vector< int >& items) const
{
	return 0;
}
	
	}
}
