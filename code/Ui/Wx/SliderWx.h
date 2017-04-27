/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_SliderWx_H
#define traktor_ui_SliderWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/ISlider.h"

namespace traktor
{
	namespace ui
	{

class SliderWx : public WidgetWxImpl< ISlider, wxSlider >
{
public:
	SliderWx(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual void setRange(int minValue, int maxValue);

	virtual void setValue(int value);

	virtual int getValue() const;

	virtual Size getPreferedSize() const;
};

	}
}

#endif	// traktor_ui_SliderWx_H
