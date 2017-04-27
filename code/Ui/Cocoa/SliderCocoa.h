/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual bool create(IWidget* parent, int style) T_OVERRIDE T_FINAL;

	virtual void setRange(int minValue, int maxValue) T_OVERRIDE T_FINAL;

	virtual void setValue(int value) T_OVERRIDE T_FINAL;

	virtual int getValue() const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_ui_SliderCocoa_H
