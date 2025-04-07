#pragma optimize("", off)

/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Win32/ToolFormWin32.h"

#include "Core/Log/Log.h"
#include "Ui/Events/NcMouseButtonDownEvent.h"
#include "Ui/Events/NcMouseButtonUpEvent.h"
#include "Ui/Events/NcMouseMoveEvent.h"
#include "Ui/Win32/BitmapWin32.h"

namespace traktor::ui
{
namespace
{

const UINT WM_ENDMODAL = WM_USER + 2000;

}

ToolFormWin32::ToolFormWin32(EventSubject* owner)
	: WidgetWin32Impl< IToolForm >(owner)
{
}

void ToolFormWin32::destroy()
{
	if (m_hMaskBitmap)
	{
		DeleteObject(m_hMaskBitmap);
		m_hMaskBitmap = 0;
	}
	WidgetWin32Impl< IToolForm >::destroy();
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
			WS_POPUP, // WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | nativeStyle,
			/*WS_EX_NOACTIVATE | */ nativeStyleEx,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			width,
			height))
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

void ToolFormWin32::setLayerImage(ISystemBitmap* layerImage)
{
	if (m_hMaskBitmap)
	{
		DeleteObject(m_hMaskBitmap);
		m_hMaskBitmap = 0;
	}

	if (layerImage)
	{
		BitmapWin32* bm = static_cast< BitmapWin32* >(layerImage);
		m_maskSize = bm->getSize();

		HDC hDCScreen = GetDC(NULL);

		BITMAPINFO bmi;
		std::memset(&bmi, 0, sizeof(bmi));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = m_maskSize.cx;
		bmi.bmiHeader.biHeight = m_maskSize.cy;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = m_maskSize.cx * m_maskSize.cy * 4;

		LPVOID bits = nullptr;
		m_hMaskBitmap = CreateDIBSection(hDCScreen, &bmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);
		std::memcpy(bits, bm->getBitsPreMulAlpha(), m_maskSize.cx * m_maskSize.cy * 4);

		ReleaseDC(NULL, hDCScreen);
	}

	updateLayerImage();
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
		if (!Application::getInstance()->process())
			break;

	if (hParentWnd)
		SetWindowPos(hParentWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

	return m_result;
}

void ToolFormWin32::endModal(DialogResult result)
{
	T_ASSERT_M(m_modal, L"Not modal");
	SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
	PostMessage(m_hWnd, WM_ENDMODAL, (WPARAM)result, 0);
}

void ToolFormWin32::setRect(const Rect& rect)
{
	WidgetWin32Impl< IToolForm >::setRect(rect);
	updateLayerImage();
}

void ToolFormWin32::updateLayerImage()
{
	const long style = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	if (!m_hMaskBitmap)
	{
		SetWindowLong(m_hWnd, GWL_EXSTYLE, style & ~WS_EX_LAYERED);
		return;
	}
	else
		SetWindowLong(m_hWnd, GWL_EXSTYLE, style | WS_EX_LAYERED);

	HDC hDCScreen = GetDC(NULL);
	HDC hDC = GetDC(m_hWnd);

	RECT rc;
	GetWindowRect(m_hWnd, &rc);

	POINT windowPosition = { rc.left, rc.top };
	SIZE windowSize = { rc.right - rc.left, rc.bottom - rc.top };
	POINT imagePosition = { 0, 0 };

	HDC hMaskDC = CreateCompatibleDC(hDC);
	HGDIOBJ oldbmp = SelectObject(hMaskDC, reinterpret_cast< HGDIOBJ >(m_hMaskBitmap));

	BLENDFUNCTION blend = { 0 };
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;
	UpdateLayeredWindow(m_hWnd, hDCScreen, &windowPosition, &windowSize, hMaskDC, &imagePosition, NULL, &blend, ULW_ALPHA);

	SelectObject(hMaskDC, oldbmp);
	DeleteDC(hMaskDC);

	ReleaseDC(NULL, hDCScreen);
	ReleaseDC(m_hWnd, hDC);
}

LRESULT ToolFormWin32::eventNcButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	int32_t button = MbtNone;
	switch (message)
	{
	case WM_NCLBUTTONDOWN:
		button = MbtLeft;
		break;
	case WM_NCMBUTTONDOWN:
		button = MbtMiddle;
		break;
	case WM_NCRBUTTONDOWN:
		button = MbtRight;
		break;
	}

	NcMouseButtonDownEvent m(
		m_owner,
		button,
		Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
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
	case WM_NCMBUTTONDOWN:
		button = MbtMiddle;
		break;
	case WM_NCRBUTTONDOWN:
		button = MbtRight;
		break;
	}

	NcMouseButtonUpEvent m(
		m_owner,
		button,
		Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
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
		Point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
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
