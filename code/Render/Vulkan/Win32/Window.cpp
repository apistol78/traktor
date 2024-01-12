/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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
{
	std::memset(&m_windowPosition, 0, sizeof(m_windowPosition));
}

Window::~Window()
{
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = 0;
	}
}

bool Window::create(uint32_t display, int32_t width, int32_t height)
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
		SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPos(m_hWnd, HWND_NOTOPMOST, m_windowPosition.x, m_windowPosition.y, 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}

	if (m_fullScreen || !IsWindowVisible(m_hWnd))
		ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);

	RECT rcWindow, rcClient;
	GetWindowRect(m_hWnd, &rcWindow);
	GetClientRect(m_hWnd, &rcClient);

	int32_t windowWidth = rcWindow.right - rcWindow.left;
	int32_t windowHeight = rcWindow.bottom - rcWindow.top;

	const int32_t realClientWidth = rcClient.right - rcClient.left;
	const int32_t realClientHeight = rcClient.bottom - rcClient.top;

	if (realClientWidth != width || realClientHeight != height)
	{
		windowWidth = (windowWidth - realClientWidth) + width;
		windowHeight = (windowHeight - realClientHeight) + height;
		SetWindowPos(m_hWnd, NULL, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);
	}

	m_fullScreen = false;
}

void Window::setFullScreenStyle(int32_t width, int32_t height)
{
	if (!m_fullScreen)
	{
		// Remember current position.
		RECT rcWindow;
		GetWindowRect(m_hWnd, &rcWindow);
		m_windowPosition.x = rcWindow.left;
		m_windowPosition.y = rcWindow.top;

		// Modify window style.
		long style = GetWindowLong(m_hWnd, GWL_STYLE);
		long styleEx = GetWindowLong(m_hWnd, GWL_EXSTYLE);

		style = style & (~WS_BORDER) & (~WS_DLGFRAME) & (~WS_THICKFRAME);
		styleEx = styleEx & (~WS_EX_WINDOWEDGE);

		SetWindowLong(m_hWnd, GWL_STYLE, style | WS_POPUP);
		SetWindowLong(m_hWnd, GWL_EXSTYLE, styleEx | WS_EX_TOPMOST);
	}

	ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);
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

void Window::addListener(IWindowListener* listener)
{
	m_listeners.insert(listener);
}

void Window::removeListener(IWindowListener* listener)
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
			for (std::set< IWindowListener* >::iterator i = window->m_listeners.begin(); i != window->m_listeners.end(); ++i)
				handled |= (*i)->windowListenerEvent(window, message, wParam, lParam, result);
		}

		if (!handled)
			result = DefWindowProc(hWnd, message, wParam, lParam);
	}

	return result;
}

}
