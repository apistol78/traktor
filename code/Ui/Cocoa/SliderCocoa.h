#ifndef traktor_ui_SliderCocoa_H
#define traktor_ui_SliderCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/ISlider.h"

namespace traktor
{
	namespace ui
	{

class SliderCocoa : public WidgetCocoaImpl< ISlider, NSSlider >
{
public:
	SliderCocoa(EventSubject* owner);
	
	// ISlider

	virtual bool create(IWidget* parent, int style);

	virtual void setRange(int minValue, int maxValue);

	virtual void setValue(int value);

	virtual int getValue() const;
};

	}
}

#endif	// traktor_ui_SliderCocoa_H
