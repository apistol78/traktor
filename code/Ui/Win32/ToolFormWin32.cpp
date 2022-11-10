/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/NcMouseButtonDownEvent.h"
#include "Ui/Events/NcMouseButtonUpEvent.h"
#include "Ui/Events/NcMouseMoveEvent.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/ToolFormWin32.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const UINT WM_ENDMODAL = WM_USER + 2000;

		}

ToolFormWin32::ToolFormWin32(EventSubject* owner)
:	WidgetWin32Impl< IToolForm >(owner)
,	m_modal(false)
,	m_result(DialogResult::Ok)
{
}

bool ToolFormWin32::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	UINT nativeStyle, nativeStyleEx;
	getNativeStyles(style, nativeStyle, nativeStyleEx);

	if (parent)
		nativeStyle |= WS_CHILD;

	if (!m_hWnd.create(
		parent ? (HWND)parent->getInternalHandle() : NULL,
		_T("TraktorWin32Class"),
		wstots(text).c_str(),
		WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | nativeStyle,
		WS_EX_NOACTIVATE | nativeStyleEx,
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
	m_hWnd.registerMessageHandler(WM_MOUSEACTIVATE, new MethodMessageHandler< ToolFormWin32 >(this, &ToolFormWin32::eventMouseActivate));
	m_hWnd.registerMessageHandler(WM_ENDMODAL, new MethodMessageHandler< ToolFormWin32 >(this, &ToolFormWin32::eventEndModal));

	m_ownCursor = true;
	return true;
}

void ToolFormWin32::setIcon(ISystemBitmap* icon)
{
	BitmapWin32* bm = static_cast< BitmapWin32* >(icon);
	HICON hIcon = bm->createIcon();
	m_hWnd.sendMessage(WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	m_hWnd.sendMessage(WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
}

DialogResult ToolFormWin32::showModal()
{
	// Ensure tool form is visible.
	setVisible(true);

	// Disable parent window, should be application main window.
	HWND hParentWnd = GetParent(m_hWnd);

	// Handle events from the dialog.
	m_result = DialogResult::Cancel;
	m_modal = true;

	while (m_modal)
	{
		if (!Application::getInstance()->process())
			break;
	}

	if (hParentWnd)
		SetWindowPos(hParentWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

	return m_result;
}

void ToolFormWin32::endModal(DialogResult result)
{
	T_ASSERT_M (m_modal, L"Not modal");
	SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
	PostMessage(m_hWnd, WM_ENDMODAL, (WPARAM)result, 0);
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

LRESULT ToolFormWin32::eventMouseActivate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	return MA_NOACTIVATE;
}

LRESULT ToolFormWin32::eventEndModal(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	m_modal = false;
	m_result = (DialogResult)wParam;
	return 0;
}

	}
}
