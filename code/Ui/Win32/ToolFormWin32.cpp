/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/NcMouseButtonDownEvent.h"
#include "Ui/Events/NcMouseButtonUpEvent.h"
#include "Ui/Events/NcMouseMoveEvent.h"
#include "Ui/Win32/ToolFormWin32.h"

namespace traktor
{
	namespace ui
	{

ToolFormWin32::ToolFormWin32(EventSubject* owner) :
	WidgetWin32Impl< IToolForm >(owner)
{
}

bool ToolFormWin32::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	UINT nativeStyle, nativeStyleEx;
	getNativeStyles(style, nativeStyle, nativeStyleEx);

	if (!m_hWnd.create(
		parent ? (HWND)parent->getInternalHandle() : NULL,
		_T("TraktorWin32Class"),
		wstots(text).c_str(),
		WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | nativeStyle,
		nativeStyleEx,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

	m_hWnd.registerMessageHandler(WM_NCLBUTTONDOWN, new MethodMessageHandler< ToolFormWin32 >(this, &ToolFormWin32::eventNcButtonDown));
	m_hWnd.registerMessageHandler(WM_NCLBUTTONUP, new MethodMessageHandler< ToolFormWin32 >(this, &ToolFormWin32::eventNcButtonUp));
	m_hWnd.registerMessageHandler(WM_NCRBUTTONDOWN, new MethodMessageHandler< ToolFormWin32 >(this, &ToolFormWin32::eventNcButtonDown));
	m_hWnd.registerMessageHandler(WM_NCRBUTTONUP, new MethodMessageHandler< ToolFormWin32 >(this, &ToolFormWin32::eventNcButtonUp));
	m_hWnd.registerMessageHandler(WM_NCMOUSEMOVE, new MethodMessageHandler< ToolFormWin32 >(this, &ToolFormWin32::eventNcMouseMove));

	return true;
}

void ToolFormWin32::center()
{
	HWND hParent = GetParent(m_hWnd);
	if (!hParent)
		hParent = GetDesktopWindow();

	RECT rcParent;
	GetWindowRect(hParent, &rcParent);

	RECT rcTool;
	GetWindowRect(m_hWnd, &rcTool);

	POINT pntPos =
	{
		rcParent.left + (rcParent.right - rcParent.left - rcTool.right + rcTool.left) / 2,
		rcParent.top + (rcParent.bottom - rcParent.top - rcTool.bottom + rcTool.top) / 2
	};
	if (pntPos.x < 0)
		pntPos.x = 0;
	if (pntPos.y < 0)
		pntPos.y = 0;

	SetWindowPos(m_hWnd, NULL, pntPos.x, pntPos.y, 0, 0, SWP_NOSIZE);
}

LRESULT ToolFormWin32::eventNcButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	int32_t button = MbtNone;
	switch (message)
	{
	case WM_NCLBUTTONDOWN:
		button = MbtLeft;
		break;
	case WM_NCRBUTTONDOWN:
		button = MbtRight;
		break;
	}

	NcMouseButtonDownEvent m(
		m_owner,
		button,
		Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
	);
	m_owner->raiseEvent(&m);

	if (!m.consumed())
		pass = true;

	return 0;
}

LRESULT ToolFormWin32::eventNcButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	int32_t button = MbtNone;
	switch (message)
	{
	case WM_NCLBUTTONDOWN:
		button = MbtLeft;
		break;
	case WM_NCRBUTTONDOWN:
		button = MbtRight;
		break;
	}

	NcMouseButtonUpEvent m(
		m_owner,
		button,
		Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
	);
	m_owner->raiseEvent(&m);

	if (!m.consumed())
		pass = true;

	return 0;
}

LRESULT ToolFormWin32::eventNcMouseMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	int32_t button = MbtNone;
	if (wParam & MK_LBUTTON)
		button |= MbtLeft;
	if (wParam & MK_MBUTTON)
		button |= MbtMiddle;
	if (wParam & MK_RBUTTON)
		button |= MbtRight;

	NcMouseMoveEvent m(
		m_owner,
		button,
		Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
	);
	m_owner->raiseEvent(&m);

	if (!m.consumed())
		pass = true;

	return 0;
}

	}
}
