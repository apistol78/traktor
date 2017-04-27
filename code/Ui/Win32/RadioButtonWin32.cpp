/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Win32/RadioButtonWin32.h"

namespace traktor
{
	namespace ui
	{

RadioButtonWin32::RadioButtonWin32(EventSubject* owner) :
	WidgetWin32Impl< IRadioButton >(owner)
{
}

bool RadioButtonWin32::create(IWidget* parent, const std::wstring& text, bool checked)
{
	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("BUTTON"),
		wstots(text).c_str(),
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_RADIOBUTTON,
		0,
		0,
		0,
		0,
		0,
		0,
		true
	))
		return false;

	if (!WidgetWin32Impl::create(0))
		return false;

	return true;
}

void RadioButtonWin32::setChecked(bool checked)
{
}

bool RadioButtonWin32::isChecked() const
{
	return false;
}

	}
}
