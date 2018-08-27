/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Gtk/SliderGtk.h"

namespace traktor
{
	namespace ui
	{

SliderGtk::SliderGtk(EventSubject* owner)
:	WidgetGtkImpl< ISlider >(owner)
{
}

bool SliderGtk::create(IWidget* parent, int style)
{
	Warp* p = static_cast< Warp* >(parent->getInternalHandle());
	T_FATAL_ASSERT(p != nullptr);

	m_warp.widget = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 10);
	gtk_container_add(GTK_CONTAINER(p->widget), m_warp.widget);

	return WidgetGtkImpl< ISlider >::create(parent);
}

void SliderGtk::setRange(int minValue, int maxValue)
{
	gtk_range_set_range(GTK_RANGE(m_warp.widget), minValue, maxValue);
}

void SliderGtk::setValue(int value)
{
	gtk_range_set_value(GTK_RANGE(m_warp.widget), value);
}

int SliderGtk::getValue() const
{
	return (int)gtk_range_get_value(GTK_RANGE(m_warp.widget));
}

	}
}
