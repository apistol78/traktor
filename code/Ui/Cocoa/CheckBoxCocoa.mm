#include "Ui/Cocoa/CheckBoxCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

namespace traktor
{
	namespace ui
	{

CheckBoxCocoa::CheckBoxCocoa(EventSubject* owner)
:	WidgetCocoaImpl< ICheckBox, NSButton >(owner)
{
}

bool CheckBoxCocoa::create(IWidget* parent, const std::wstring& text, bool checked)
{
	m_control = [[NSButton alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setButtonType: NSSwitchButton];
	[m_control setTitle: makeNSString(text)];
	[m_control setTarget: NSApp];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];
	
	return true;
}

void CheckBoxCocoa::setChecked(bool checked)
{
}

bool CheckBoxCocoa::isChecked() const
{
	return false;
}

Size CheckBoxCocoa::getPreferedSize() const
{
	return Size(200, 32);
}

	}
}
