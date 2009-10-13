#include "Ui/Cocoa/DropDownCocoa.h"

namespace traktor
{
	namespace ui
	{

DropDownCocoa::DropDownCocoa(EventSubject* owner)
:	WidgetCocoaImpl< IDropDown, NSComboBox >(owner)
{
}

bool DropDownCocoa::create(IWidget* parent, const std::wstring& text, int style)
{
//	NSListDataSource* dataSource = [[[NSListDataSource alloc] init] autorelease];
//	[dataSource setCallback: this];

	m_control = [[NSComboBox alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setTarget: NSApp];
//	[m_control setDataSource: dataSource];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];
	
	return true;
}

int DropDownCocoa::add(const std::wstring& item)
{
	return 0;
}

bool DropDownCocoa::remove(int index)
{
	return false;
}

void DropDownCocoa::removeAll()
{
}

int DropDownCocoa::count() const
{
	return 0;
}

std::wstring DropDownCocoa::get(int index) const
{
	return L"";
}

void DropDownCocoa::select(int index)
{
}

int DropDownCocoa::getSelected() const	
{
	return -1;
}
	
	}
}
