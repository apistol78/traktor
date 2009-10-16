#include "Ui/Cocoa/SliderCocoa.h"

namespace traktor
{
	namespace ui
	{

SliderCocoa::SliderCocoa(EventSubject* owner)
:	WidgetCocoaImpl< ISlider, NSSlider >(owner)
{
}

bool SliderCocoa::create(IWidget* parent, int style)
{
	m_control = [[NSSlider alloc]
		initWithFrame: NSMakeRect(0, 0, 100, 10)
	];
	[m_control setEnabled: YES];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];

	return true;
}

void SliderCocoa::setRange(int minValue, int maxValue)
{
	[m_control setMinValue: (double)minValue];
	[m_control setMaxValue: (double)maxValue];
}

void SliderCocoa::setValue(int value)
{
	[m_control setDoubleValue: (double)value];
}

int SliderCocoa::getValue() const
{
	return (int)[m_control doubleValue];
}
	
	}
}
