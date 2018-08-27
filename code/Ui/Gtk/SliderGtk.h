/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_SliderGtk_H
#define traktor_ui_SliderGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/ISlider.h"

namespace traktor
{
	namespace ui
	{

class SliderGtk : public WidgetGtkImpl< ISlider >
{
public:
	SliderGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual void setRange(int minValue, int maxValue);

	virtual void setValue(int value);

	virtual int getValue() const;
};

	}
}

#endif	// traktor_ui_SliderGtk_H
