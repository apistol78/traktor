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
	NSComboBoxDataSource* dataSource = [[NSComboBoxDataSource alloc] init];
	[dataSource setCallback: this];

	m_control = [[NSComboBox alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setUsesDataSource: YES];
	[m_control setDataSource: dataSource];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];
	
	return true;
}

int DropDownCocoa::add(const std::wstring& item)
{
	m_items.push_back(item);
	[m_control reloadData];
	return int(m_items.size() - 1);
}

bool DropDownCocoa::remove(int index)
{
	[m_control reloadData];
	return true;
}

void DropDownCocoa::removeAll()
{
	m_items.resize(0);
	[m_control reloadData];
}

int DropDownCocoa::count() const
{
	return int(m_items.size());
}

std::wstring DropDownCocoa::get(int index) const
{
	return m_items[index];
}

void DropDownCocoa::select(int index)
{
	[m_control selectItemAtIndex: index];
}

int DropDownCocoa::getSelected() const	
{
	return [m_control indexOfSelectedItem];
}

int DropDownCocoa::comboBox_indexOf(const std::wstring& value) const
{
	std::vector< std::wstring >::const_iterator i = std::find(m_items.begin(), m_items.end(), value);
	if (i != m_items.end())
		return (int)std::distance(m_items.begin(), i);
	else
		return -1;
}

std::wstring DropDownCocoa::comboBox_valueAt(int index) const
{
	return m_items[index];
}

int DropDownCocoa::comboBox_numberOfItems() const
{
	return int(m_items.size());
}

	}
}
