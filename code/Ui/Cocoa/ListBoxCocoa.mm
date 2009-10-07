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
	m_control = [[NSTableView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setTarget: NSApp];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];
	
	return true;
}

int ListBoxCocoa::add(const std::wstring& item)
{
	return 0;
}

bool ListBoxCocoa::remove(int index)
{
	return true;
}

void ListBoxCocoa::removeAll()
{
}

int ListBoxCocoa::count() const
{
	return [m_control numberOfRows];
}

void ListBoxCocoa::set(int index, const std::wstring& item)
{
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
