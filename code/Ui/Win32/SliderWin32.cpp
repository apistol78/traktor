/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Slider.h"
#include "Ui/Win32/SliderWin32.h"

namespace traktor
{
	namespace ui
	{

SliderWin32::SliderWin32(EventSubject* owner) :
	WidgetWin32Impl< ISlider >(owner)
{
}

bool SliderWin32::create(IWidget* parent, int style)
{
	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		TRACKBAR_CLASS,
		NULL,
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | TBS_AUTOTICKS | TBS_HORZ | TBS_BOTTOM,
		0,
		0,
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		true
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

	m_hWnd.registerMessageHandler(WM_REFLECTED_HSCROLL, new MethodMessageHandler< SliderWin32 >(this, &SliderWin32::eventScroll));
	m_hWnd.registerMessageHandler(WM_REFLECTED_VSCROLL, new MethodMessageHandler< SliderWin32 >(this, &SliderWin32::eventScroll));

	return true;
}

void SliderWin32::setRange(int minValue, int maxValue)
{
	int pageSize = (maxValue - minValue) / 20;
	m_hWnd.sendMessage(TBM_SETRANGEMIN, FALSE, minValue);
	m_hWnd.sendMessage(TBM_SETRANGEMAX, TRUE, maxValue);
	m_hWnd.sendMessage(TBM_SETPAGESIZE, 0, (LPARAM)pageSize);
	m_hWnd.sendMessage(TBM_SETTICFREQ, (WPARAM)pageSize, 0);
}

void SliderWin32::setValue(int value)
{
	m_hWnd.sendMessage(TBM_SETPOS, TRUE, (LPARAM)value);
}

int SliderWin32::getValue() const
{
	return (int)m_hWnd.sendMessage(TBM_GETPOS, 0, 0);
}

Size SliderWin32::getPreferedSize() const
{
	int32_t dpi = m_hWnd.getSystemDPI();
	return Size(
		(200 * dpi) / 96,
		(32 * dpi) / 96
	);
}

LRESULT SliderWin32::eventScroll(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	ContentChangeEvent contentChangeEvent(m_owner);
	m_owner->raiseEvent(&contentChangeEvent);
	return TRUE;
}

	}
}
