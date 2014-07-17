#include "Ui/Cocoa/CheckBoxCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Events/ButtonClickEvent.h"
#include "Ui/EventSubject.h"

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
	NSTargetProxy* targetProxy = [[NSTargetProxy alloc] init];
	[targetProxy setCallback: this];

	m_control = [[NSButton alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setButtonType: NSSwitchButton];
	[m_control setTitle: makeNSString(text)];
	[m_control setState: checked ? NSOnState : NSOffState];
	[m_control setTarget: targetProxy];
	[m_control setAction: @selector(dispatchActionCallback:)];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];
	
	return true;
}

void CheckBoxCocoa::setChecked(bool checked)
{
	[m_control setState: checked ? NSOnState : NSOffState];
}

bool CheckBoxCocoa::isChecked() const
{
	return [m_control state] == NSOnState;
}

void CheckBoxCocoa::targetProxy_Action(void* controlId)
{
	ButtonClickEvent clickEvent(m_owner);
	m_owner->raiseEvent(&clickEvent);
}

void CheckBoxCocoa::targetProxy_doubleAction(void* controlId)
{
}

	}
}
