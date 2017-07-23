/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Win32/StaticWin32.h"

namespace traktor
{
	namespace ui
	{

StaticWin32::StaticWin32(EventSubject* owner)
:	WidgetWin32Impl< IStatic >(owner)
{
}

bool StaticWin32::create(IWidget* parent, const std::wstring& text)
{
	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("STATIC"),
		wstots(text).c_str(),
		WS_VISIBLE | WS_CHILD | SS_LEFT | SS_NOTIFY,
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

Size StaticWin32::getPreferedSize() const
{
	SIZE size = { 0, 0 };

	std::wstring text = getText();
	if (!text.empty())
	{
		HDC hDC = GetDC(m_hWnd);
		HGDIOBJ hOldFont = SelectObject(hDC, m_hWnd.getFont());
		GetTextExtentPoint32(hDC, wstots(text).c_str(), int(text.length()), &size);
		SelectObject(hDC, hOldFont);
		ReleaseDC(m_hWnd, hDC);
	}

	return Size(size.cx, size.cy);
}

Size StaticWin32::getMaximumSize() const
{
	return getPreferedSize();
}

	}
}
