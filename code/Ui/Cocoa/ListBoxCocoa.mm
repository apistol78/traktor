#include "Ui/EventSubject.h"
#include "Ui/Cocoa/ListBoxCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Events/MouseDoubleClickEvent.h"

namespace traktor
{
	namespace ui
	{
	
ListBoxCocoa::ListBoxCocoa(EventSubject* owner)
:	WidgetCocoaImpl< IListBox, NSTableView, NSScrollView >(owner)
{
}

bool ListBoxCocoa::create(IWidget* parent, int style)
{
	NSTargetProxy* targetProxy = [[NSTargetProxy alloc] init];
	[targetProxy setCallback: this];

	m_view = [[NSScrollView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_view setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
	[m_view setHasVerticalScroller: YES];
	[m_view setHasHorizontalScroller: YES];
	
	NSListDataSource* dataSource = [[NSListDataSource alloc] init];
	[dataSource setCallback: this];

	NSTableColumn* column = [[NSTableColumn alloc] initWithIdentifier: nil];
	[column setEditable: NO];
	
	NSCell* dataCell = [column dataCell];
	[dataCell setFont: [NSFont controlContentFontOfSize: 11]];

	m_control = [[NSTableView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setColumnAutoresizingStyle: NSTableViewUniformColumnAutoresizingStyle];
	[m_control addTableColumn: column];
	[m_control setTarget: targetProxy];
	[m_control setAction: @selector(dispatchActionCallback:)];
	[m_control setDoubleAction: @selector(dispatchDoubleActionCallback:)];
	[m_control setHeaderView: nil];
	[m_control setDataSource: dataSource];
	
	[m_view setDocumentView: m_control];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_view];
	
	return class_t::create();
}

int ListBoxCocoa::add(const std::wstring& item)
{
	m_items.push_back(item);
	[m_control reloadData];
	return int(m_items.size() - 1);
}

bool ListBoxCocoa::remove(int index)
{
	if (index < 0 || index >= int(m_items.size()))
		return false;
		
	m_items.erase(m_items.begin() + index);
	
	[m_control reloadData];
	return true;
}

void ListBoxCocoa::removeAll()
{
	m_items.resize(0);
	[m_control reloadData];
}

int ListBoxCocoa::count() const
{
	return int(m_items.size());
}

void ListBoxCocoa::set(int index, const std::wstring& item)
{
	[m_control reloadData];
}

std::wstring ListBoxCocoa::get(int index) const
{
	return m_items[index];
}

void ListBoxCocoa::select(int index)
{
	[m_control selectRow: index byExtendingSelection: NO];
}

bool ListBoxCocoa::selected(int index) const
{
	return [m_control isRowSelected: index] == YES;
}

Rect ListBoxCocoa::getItemRect(int index) const
{
	NSRect rc = [m_control rectOfRow: index];
	return fromNSRect(rc);
}

int ListBoxCocoa::listCount() const
{
	return int(m_items.size());
}

std::wstring ListBoxCocoa::listValue(NSTableColumn* tableColumn, int index) const
{
	return m_items[index];
}

void ListBoxCocoa::targetProxy_Action(void* controlId)
{
}

void ListBoxCocoa::targetProxy_doubleAction(void* controlId)
{
	NSWindow* window = [m_control window];
	NSPoint mousePosition = [window mouseLocationOutsideOfEventStream];
	mousePosition = [m_control convertPointFromBase: mousePosition];

	MouseDoubleClickEvent mouseEvent(
		m_owner,
		MbtLeft,
		fromNSPoint(mousePosition)
	);
	m_owner->raiseEvent(&mouseEvent);
}

	}
}
