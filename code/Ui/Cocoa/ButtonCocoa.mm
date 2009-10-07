#include "Ui/Cocoa/ButtonCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

namespace traktor
{
	namespace ui
	{

ButtonCocoa::ButtonCocoa(EventSubject* owner)
:	WidgetCocoaImpl< IButton, NSButton >(owner)
{
}

bool ButtonCocoa::create(IWidget* parent, const std::wstring& text, int style)
{
	m_control = [[NSButton alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setBezelStyle: NSRoundedBezelStyle];
	[m_control setTitle: makeNSString(text)];
	[m_control setTarget: NSApp];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];
	
	return true;
}

void ButtonCocoa::setState(bool state)
{
}

bool ButtonCocoa::getState() const
{
	return false;
}

Size ButtonCocoa::getPreferedSize() const
{
	return Size(200, 32);
}

	}
}
