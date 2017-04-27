/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Win32/NativeWin32.h"

namespace traktor
{
	namespace ui
	{ 

NativeWin32::NativeWin32(EventSubject* owner)
:	WidgetWin32Impl(owner)
{
}

bool NativeWin32::create(void* nativeHandle)
{
	RECT rc;
	GetClientRect((HWND)nativeHandle, &rc);

	if (!m_hWnd.create(
		(HWND)nativeHandle,
		_T("TraktorWin32Class"),
		_T(""),
		WS_CHILD | WS_VISIBLE,
		0,
		rc.left,
		rc.top,
		rc.right - rc.left,
		rc.bottom - rc.top
	))
		return false;

	if (!WidgetWin32Impl::create(0))
		return false;

	return true;
}

	}
}
