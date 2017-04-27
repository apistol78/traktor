/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Wx/SliderWx.h"

namespace traktor
{
	namespace ui
	{

SliderWx::SliderWx(EventSubject* owner)
:	WidgetWxImpl< ISlider, wxSlider >(owner)
{
}

bool SliderWx::create(IWidget* parent, int style)
{
	m_window = new wxSlider();

	if (!m_window->Create(
		static_cast< wxWindow* >(parent->getInternalHandle()),
		-1,
		0,
		0,
		100
	))
	{
		m_window->Destroy();
		m_window = 0;
		return false;
	}

	if (!WidgetWxImpl< ISlider, wxSlider >::create(0))
		return false;

	return true;
}

void SliderWx::setRange(int minValue, int maxValue)
{
	m_window->SetRange(minValue, maxValue);
}

void SliderWx::setValue(int value)
{
	m_window->SetValue(value);
}

int SliderWx::getValue() const
{
	return m_window->GetValue();
}

Size SliderWx::getPreferedSize() const
{
	return Size(200, 32);
}

	}
}
