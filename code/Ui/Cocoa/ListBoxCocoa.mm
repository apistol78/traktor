#import "Ui/Cocoa/NSListDataSource.h"

#include "Ui/Cocoa/ListBoxCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

namespace traktor
{
	namespace ui
	{
	
ListBoxCocoa::ListBoxCocoa(EventSubject* owner)
:	WidgetCocoaImpl< IListBox, NSTableView >(owner)
{
}

bool ListBoxCocoa::create(IWidget* parent, int style)
{
	NSListDataSource* dataSource = [[NSListDataSource alloc] init];

	NSTableColumn* column = [[NSTableColumn alloc] initWithIdentifier: nil];

	m_control = [[NSTableView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setColumnAutoresizingStyle: NSTableViewUniformColumnAutoresizingStyle];
	[m_control addTableColumn: column];
	[m_control setTarget: NSApp];
	[m_control setDataSource: dataSource];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];
	
	return true;
}

int ListBoxCocoa::add(const std::wstring& item)
{
	[m_control reloadData];
	return 0;
}

bool ListBoxCocoa::remove(int index)
{
	[m_control reloadData];
	return true;
}

void ListBoxCocoa::removeAll()
{
	[m_control reloadData];
}

int ListBoxCocoa::count() const
{
	return [m_control numberOfRows];
}

void ListBoxCocoa::set(int index, const std::wstring& item)
{
	[m_control reloadData];
}

std::wstring ListBoxCocoa::get(int index) const
{
	return L"";
}

void ListBoxCocoa::select(int index)
{
}

bool ListBoxCocoa::selected(int index) const
{
	return [m_control isRowSelected: index] == YES;
}

Rect ListBoxCocoa::getItemRect(int index) const
{
	return Rect(0, 0, 0, 0);
}

	}
}
