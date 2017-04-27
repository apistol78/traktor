/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Layout.h"
#include "Ui/Native.h"
#include "Ui/Itf/INative.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Native", Native, Widget)

bool Native::create(void* nativeHandle, Layout* layout)
{
	m_layout = layout;

	INative* native = Application::getInstance()->getWidgetFactory()->createNative(this);
	if (!native)
	{
		log::error << L"Failed to create native widget peer (Native)" << Endl;
		return false;
	}

	if (!native->create(nativeHandle))
	{
		native->destroy();
		return false;
	}

	m_widget = native;

	addEventHandler< SizeEvent >(this, &Native::eventSize);

	return Widget::create(0);
}

void Native::update(const Rect* rc, bool immediate)
{
	if (m_layout)
		m_layout->update(this);

	Widget::update(rc, immediate);
}

void Native::eventSize(SizeEvent* event)
{
	update(0, false);
}

	}
}
