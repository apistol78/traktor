#include "Ui/Cocoa/ButtonCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Events/ButtonClickEvent.h"
#include "Ui/EventSubject.h"

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
	NSTargetProxy* targetProxy = [[NSTargetProxy alloc] init];
	[targetProxy setCallback: this];

	m_control = [[NSButton alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setBezelStyle: NSRoundedBezelStyle];
	[m_control setTitle: makeNSString(text)];
	[m_control setTarget: targetProxy];
	[m_control setAction: @selector(dispatchActionCallback:)];
	
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

void ButtonCocoa::targetProxy_Action(void* controlId)
{
	ButtonClickEvent clickEvent(m_owner);
	m_owner->raiseEvent(&clickEvent);
}

void ButtonCocoa::targetProxy_doubleAction(void* controlId)
{
}

	}
}
