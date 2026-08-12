/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <vector>
#include "Render/Vulkan/Win32/Window.h"

namespace traktor::render
{
	namespace
	{

const TCHAR* c_className = _T("TraktorRenderSystemVulkan");
const DWORD c_classIconResource = 10000;

BOOL enumMonitors(HMONITOR hMonitor, HDC hDC, LPRECT lpRect, LPARAM lpUser)
{
	std::vector< HMONITOR >* monitors = (std::vector< HMONITOR >*)lpUser;
	monitors->push_back(hMonitor);
	return TRUE;
}

	}

Window::Window()
:	m_hWnd(0)
,	m_fullScreen(false)
,	m_windowedStyle(WS_OVERLAPPEDWINDOW)
,	m_windowedStyleEx(0)
{
	std::memset(&m_windowedRect, 0, sizeof(m_windowedRect));
}

Window::~Window()
{
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = 0;
	}
}

bool Window::create(uint32_t display, int32_t width, int32_t height, bool fullscreen)
{
	T_ASSERT(!m_hWnd);

	WNDCLASS wc;
	std::memset(&wc, 0, sizeof(wc));
	wc.style = 0;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(this);
	wc.lpfnWndProc = (WNDPROC)wndProc;
	wc.hInstance = static_cast< HINSTANCE >(GetModuleHandle(NULL));
	wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(c_classIconResource));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = c_className;
	RegisterClass(&wc);

	int positionX = CW_USEDEFAULT;
	int positionY = CW_USEDEFAULT;

	std::vector< HMONITOR > monitors;
	EnumDisplayMonitors(NULL, NULL, &enumMonitors, (LPARAM)&monitors);
	if (display < monitors.size())
	{
		MONITORINFO mi = {};
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(monitors[display], &mi);

		const auto& rc = mi.rcWork;
		positionX = rc.left + ((rc.right - rc.left) - width) / 2;
		positionY = rc.top + ((rc.bottom - rc.top) - height) / 2;
	}

	m_hWnd = CreateWindow(
		c_className,
		L"",
		WS_OVERLAPPEDWINDOW,
		positionX,
		positionY,
		width,
		height,
		NULL,
		NULL,
		static_cast< HMODULE >(GetModuleHandle(NULL)),
		this
	);
	if (!m_hWnd)
		return false;

	if (fullscreen)
		setFullScreenStyle(width, height);
	else
		setWindowedStyle(width, height);

	return true;
}

void Window::setTitle(const wchar_t* title)
{
	SetWindowText(m_hWnd, title ? title : L"");
}

void Window::setWindowedStyle(int32_t width, int32_t height)
{
	if (m_fullScreen)
	{
		// Restore the presentation saved on the way into fullscreen; the whole
		// rectangle, as the fullscreen window owns the size as well as the position.
		// The requested client size is applied below and wins when it is valid, so
		// restoring the size here only matters when the caller has none to give.
		SetWindowLong(m_hWnd, GWL_STYLE, m_windowedStyle);
		SetWindowLong(m_hWnd, GWL_EXSTYLE, m_windowedStyleEx);

		SetWindowPos(
			m_hWnd,
			HWND_NOTOPMOST,
			m_windowedRect.left,
			m_windowedRect.top,
			m_windowedRect.right - m_windowedRect.left,
			m_windowedRect.bottom - m_windowedRect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		// Cursor visibility belongs to the render view, which answers WM_SETCURSOR
		// from its own state; forcing an arrow here would undo hideCursor().
	}

	if (m_fullScreen || !IsWindowVisible(m_hWnd))
		ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);

	// Nothing sensible to size to; keep whatever was restored above.
	if (width <= 0 || height <= 0)
	{
		m_fullScreen = false;
		return;
	}

	RECT rcWindow, rcClient;
	GetWindowRect(m_hWnd, &rcWindow);
	GetClientRect(m_hWnd, &rcClient);

	int32_t realClientWidth = rcClient.right - rcClient.left;
	int32_t realClientHeight = rcClient.bottom - rcClient.top;

	if (realClientWidth != width || realClientHeight != height)
	{
		if (IsZoomed(m_hWnd))
		{
			ShowWindow(m_hWnd, SW_RESTORE);
			GetWindowRect(m_hWnd, &rcWindow);
			GetClientRect(m_hWnd, &rcClient);
			realClientWidth = rcClient.right - rcClient.left;
			realClientHeight = rcClient.bottom - rcClient.top;
		}

		if (realClientWidth != width || realClientHeight != height)
		{
			const int32_t windowWidth = (rcWindow.right - rcWindow.left) - realClientWidth + width;
			const int32_t windowHeight = (rcWindow.bottom - rcWindow.top) - realClientHeight + height;
			SetWindowPos(m_hWnd, NULL, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);
		}
	}

	m_fullScreen = false;
}

void Window::setFullScreenStyle(int32_t width, int32_t height)
{
	if (!m_fullScreen)
	{
		// Remember the windowed presentation in full so it can be restored later.
		GetWindowRect(m_hWnd, &m_windowedRect);
		m_windowedStyle = GetWindowLong(m_hWnd, GWL_STYLE);
		m_windowedStyleEx = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	}

	// A minimized window cannot be repositioned; restore it before moving it.
	if (IsIconic(m_hWnd))
		ShowWindow(m_hWnd, SW_RESTORE);

	// Strip the frame and make it a popup. WS_SYSMENU is deliberately kept so Alt+F4
	// still closes the window while fullscreen.
	SetWindowLong(m_hWnd, GWL_STYLE, (m_windowedStyle & ~(WS_CAPTION | WS_BORDER | WS_DLGFRAME | WS_THICKFRAME | WS_MAXIMIZE | WS_MINIMIZE)) | WS_POPUP);
	SetWindowLong(m_hWnd, GWL_EXSTYLE, m_windowedStyleEx & ~(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME));

	// Cover exactly one monitor. Maximizing a WS_POPUP window is not reliably the
	// same thing; depending on the styles left on the window it can size to the work
	// area instead, which leaves the task bar showing over the view. The monitor
	// rectangle is therefore applied explicitly. The requested size is only a fall
	// back, as borderless fullscreen never changes the display mode.
	RECT rcTarget = { 0, 0, width, height };
	MONITORINFO mi = {};
	mi.cbSize = sizeof(mi);
	if (GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &mi))
		rcTarget = mi.rcMonitor;

	// Placed at the top of the normal Z order rather than made topmost; a topmost
	// window stays over everything else after the user has alt-tabbed away, and a
	// borderless window covering the whole monitor already hides the task bar while
	// it is in the foreground.
	SetWindowPos(
		m_hWnd,
		HWND_TOP,
		rcTarget.left,
		rcTarget.top,
		rcTarget.right - rcTarget.left,
		rcTarget.bottom - rcTarget.top,
		SWP_FRAMECHANGED | SWP_NOACTIVATE);

	if (!IsWindowVisible(m_hWnd))
		ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);

	m_fullScreen = true;
}

void Window::show()
{
	ShowWindow(m_hWnd, SW_SHOW);
}

void Window::hide()
{
	ShowWindow(m_hWnd, SW_HIDE);
}

bool Window::isActive() const
{
	return GetForegroundWindow() == m_hWnd;
}

int32_t Window::getWidth() const
{
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);
	return (int32_t)(rcClient.right - rcClient.left);
}

int32_t Window::getHeight() const
{
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);
	return (int32_t)(rcClient.bottom - rcClient.top);
}

uint32_t Window::getDisplay() const
{
	HMONITOR monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);

	std::vector< HMONITOR > monitors;
	EnumDisplayMonitors(NULL, NULL, &enumMonitors, (LPARAM)&monitors);

	auto it = std::find(monitors.begin(), monitors.end(), monitor);
	return (uint32_t)std::distance(monitors.begin(), it);
}

Window::operator HWND () const
{
	return m_hWnd;
}

void Window::addListener(IListener* listener)
{
	m_listeners.insert(listener);
}

void Window::removeListener(IListener* listener)
{
	m_listeners.erase(listener);
}

LRESULT CALLBACK Window::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = FALSE;
	Window* window = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT createStruct = reinterpret_cast< LPCREATESTRUCT >(lParam);
		window = reinterpret_cast< Window* >(createStruct->lpCreateParams);
		SetWindowLongPtr(hWnd, 0, reinterpret_cast< LONG_PTR >(window));
	}
	else
	{
		bool handled = false;

		window = reinterpret_cast< Window* >(GetWindowLongPtr(hWnd, 0));
		if (window)
		{
			for (const auto& listener : window->m_listeners)
				handled |= listener->windowListenerEvent(window, message, wParam, lParam, result);
		}

		if (!handled)
			result = DefWindowProc(hWnd, message, wParam, lParam);
	}

	return result;
}

}
