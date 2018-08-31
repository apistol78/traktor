/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Slider.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Slider", Slider, Widget)

bool Slider::create(Widget* parent, int style)
{
	if (!Widget::create(parent))
		return false;

	//addEventHandler< PaintEvent >(this, &Button::eventPaint);
	return true;
}

void Slider::setRange(int minValue, int maxValue)
{
}

void Slider::setValue(int value)
{
}

int Slider::getValue() const
{
	return 0;
}

	}
}
