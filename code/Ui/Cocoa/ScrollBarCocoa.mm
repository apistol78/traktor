#include "Ui/Cocoa/ScrollBarCocoa.h"
#include "Ui/ScrollBar.h"

namespace traktor
{
	namespace ui
	{
	
ScrollBarCocoa::ScrollBarCocoa(EventSubject* owner)
:	WidgetCocoaImpl< IScrollBar, NSScroller >(owner)
,	m_vertical(false)
,	m_range(100)
,	m_page(10)
{
}

bool ScrollBarCocoa::create(IWidget* parent, int style)
{
	m_vertical = (style & ScrollBar::WsVertical) != 0;

	m_control = [[NSScroller alloc]
		initWithFrame: m_vertical ? NSMakeRect(0, 0, 10, 100) : NSMakeRect(0, 0, 100, 10)
	];
	[m_control setEnabled: YES];
	
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
	m_page = page;
}

int ScrollBarCocoa::getPage() const
{
	return m_page;
}

void ScrollBarCocoa::setPosition(int position)
{
	float knobPosition = float(position) / m_range;
	float knobProp = float(m_page) / m_range;
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
	
	if (m_vertical)
		preferedSize.cx = 20;
	else
		preferedSize.cy = 20;

	return preferedSize;
}
	
	}
}
