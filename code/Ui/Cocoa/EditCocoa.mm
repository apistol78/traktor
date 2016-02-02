#include "Ui/Cocoa/EditCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{

EditCocoa::EditCocoa(EventSubject* owner)
:	WidgetCocoaImpl< IEdit, NSTextField >(owner)
{
}

bool EditCocoa::create(IWidget* parent, const std::wstring& text, int style)
{
	m_control = [[NSTextField alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setEditable: YES];
	[m_control setBezeled: YES];
	[m_control setStringValue: makeNSString(text)];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];
	
	return class_t::create();
}

void EditCocoa::setSelection(int from, int to)
{
}

void EditCocoa::getSelection(int& outFrom, int& outTo) const
{
}

void EditCocoa::selectAll()
{
	[m_control selectText: nil];
}

void EditCocoa::setBorderColor(const Color4ub& borderColor)
{
}

void EditCocoa::setText(const std::wstring& text)
{
	[m_control setStringValue: makeNSString(text)];
}

std::wstring EditCocoa::getText() const
{
	NSString* s = [m_control stringValue];
	return fromNSString(s);
}

	}
}
