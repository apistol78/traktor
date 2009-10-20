#include "Ui/Cocoa/StaticCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

namespace traktor
{
	namespace ui
	{

StaticCocoa::StaticCocoa(EventSubject* owner)
:	WidgetCocoaImpl< IStatic, NSTextField >(owner)
{
}

bool StaticCocoa::create(IWidget* parent, const std::wstring& text)
{
	m_control = [[NSTextField alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_control setEditable: NO];
	[m_control setBordered: NO];
	[m_control setBezeled: NO];
	[m_control setDrawsBackground: NO];
	[m_control setStringValue: makeNSString(text)];

	NSCell* cell = [m_control cell];
	[cell setLineBreakMode: NSLineBreakByClipping];

	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];
	
	return true;
}

	}
}
