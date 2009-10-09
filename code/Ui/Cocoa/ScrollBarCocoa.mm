#include "Ui/Cocoa/ScrollBarCocoa.h"

namespace traktor
{
	namespace ui
	{
	
ScrollBarCocoa::ScrollBarCocoa(EventSubject* owner)
:	WidgetCocoaImpl< IScrollBar, NSScroller >(owner)
,	m_range(100)
{
}

bool ScrollBarCocoa::create(IWidget* parent, int style)
{
	m_control = [[NSScroller alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];

	return true;
}

void ScrollBarCocoa::setRange(int range)
{
	m_range = range;
}

int ScrollBarCocoa::getRange() const
{
	return m_range;
}

void ScrollBarCocoa::setPage(int page)
{
}

int ScrollBarCocoa::getPage() const
{
	return 0;
}

void ScrollBarCocoa::setPosition(int position)
{
	float knobPosition = float(position) / m_range;
	float knobProp = 0.1f;
	[m_control setFloatValue: knobPosition knobProportion: knobProp];
}

int ScrollBarCocoa::getPosition() const
{
	float knobPosition = [m_control floatValue];
	return int(knobPosition * m_range);
}

Size ScrollBarCocoa::getPreferedSize() const
{
	Size preferedSize = WidgetCocoaImpl< IScrollBar, NSScroller >::getPreferedSize();
	preferedSize.cx = 200;
	preferedSize.cy = 20;
	return preferedSize;
}
	
	}
}
