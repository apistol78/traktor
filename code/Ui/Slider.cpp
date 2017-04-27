/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Slider.h"
#include "Ui/Itf/ISlider.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Slider", Slider, Widget)

bool Slider::create(Widget* parent, int style)
{
	if (!parent)
		return false;

	ISlider* slider = Application::getInstance()->getWidgetFactory()->createSlider(this);
	if (!slider)
	{
		log::error << L"Failed to create native widget peer (Slider)" << Endl;
		return false;
	}

	if (!slider->create(parent->getIWidget(), style))
	{
		slider->destroy();
		return false;
	}

	m_widget = slider;

	return Widget::create(parent);
}

void Slider::setRange(int minValue, int maxValue)
{
	T_ASSERT (m_widget);
	static_cast< ISlider* >(m_widget)->setRange(minValue, maxValue);
}

void Slider::setValue(int value)
{
	T_ASSERT (m_widget);
	static_cast< ISlider* >(m_widget)->setValue(value);
}

int Slider::getValue() const
{
	T_ASSERT (m_widget);
	return static_cast< ISlider* >(m_widget)->getValue();
}

	}
}
