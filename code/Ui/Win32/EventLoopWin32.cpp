/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/SmallMap.h"
#include "Core/Log/Log.h"
#include "Ui/Enums.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/FocusEvent.h"
#include "Ui/Events/IdleEvent.h"
#include "Ui/Events/MouseButtonDownEvent.h"
#include "Ui/Events/MouseButtonUpEvent.h"
#include "Ui/Events/MouseMoveEvent.h"
#include "Ui/Events/KeyDownEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/KeyUpEvent.h"
#if defined(T_USE_DIRECT2D)
#	include "Ui/Win32/CanvasDirect2DWin32.h"
#endif
#if defined(T_USE_GDI_PLUS)
#	include "Ui/Win32/CanvasGdiPlusWin32.h"
#endif
#include "Ui/Win32/EventLoopWin32.h"
#include "Ui/Win32/UtilitiesWin32.h"

HINSTANCE g_hInstance = NULL;

namespace traktor::ui
{
namespace
{

// Thread timers for the callback timer service; keyed by the native timer id
// so the trampoline can find the callback. UI thread only.
SmallMap< UINT_PTR, std::function< void() > > s_timerCallbacks;

void CALLBACK timerCallbackProc(HWND, UINT, UINT_PTR id, DWORD)
{
	const auto it = s_timerCallbacks.find(id);
	if (it != s_timerCallbacks.end())
	{
		// Copy as the callback may start or stop timers.
		const std::function< void() > fn = it->second;
		fn();
	}
}

// Windows delivers wheel messages to the keyboard focus window; reroute them
// to the window beneath the pointer so scrolling affects what is pointed at,
// e.g. a hovered 3d viewport rather than a graph keeping focus in a sibling
// pane. Only enabled windows on this thread are valid targets, and an active
// mouse capture keeps the focus routing since the captured window is the one
// being operated.
void routeMouseWheelToPointer(MSG& msg)
{
	if (msg.message != WM_MOUSEWHEEL && msg.message != WM_MOUSEHWHEEL)
		return;
	if (GetCapture() != NULL)
		return;

	const POINT pnt = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
	const HWND hWndUnder = WindowFromPoint(pnt);
	if (hWndUnder == NULL || hWndUnder == msg.hwnd)
		return;

	if (GetWindowThreadProcessId(hWndUnder, NULL) == GetCurrentThreadId() && IsWindowEnabled(hWndUnder))
		msg.hwnd = hWndUnder;
}

}

EventLoopWin32::EventLoopWin32()
:	m_exitCode(0)
,	m_terminate(false)
{
#if defined(T_STATIC)
	g_hInstance = (HINSTANCE)GetModuleHandle(NULL);
#endif

	OleInitialize(NULL);
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

#if defined(T_USE_DIRECT2D)
	CanvasDirect2DWin32::startup();
#endif
#if defined(T_USE_GDI_PLUS)
	CanvasGdiPlusWin32::startup();
#endif

	Window::registerDefaultClass();
	Window::registerDialogClass();
}

EventLoopWin32::~EventLoopWin32()
{
	Window::unregisterDialogClass();
	Window::unregisterDefaultClass();

#if defined(T_USE_GDI_PLUS)
	CanvasGdiPlusWin32::shutdown();
#endif
#if defined(T_USE_DIRECT2D)
	CanvasDirect2DWin32::shutdown();
#endif

#if !defined(WINCE)
	OleUninitialize();
#endif
}

void EventLoopWin32::destroy()
{
	delete this;
}

bool EventLoopWin32::process(EventSubject* owner)
{
	if (m_terminate)
		return false;

	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		routeMouseWheelToPointer(msg);

		const bool dispatch = !preTranslateMessage(owner, msg);
		if (dispatch)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return true;
}

int32_t EventLoopWin32::execute(EventSubject* owner)
{
	while (!m_terminate)
	{
		MSG msg = { 0 };
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			routeMouseWheelToPointer(msg);

			const bool dispatch = !preTranslateMessage(owner, msg);
			if (dispatch)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			continue;
		}

		IdleEvent idleEvent(owner);
		owner->raiseEvent(&idleEvent);
		if (!idleEvent.requestedMore())
			WaitMessage();
	}

	return m_exitCode;
}

void EventLoopWin32::exit(int32_t exitCode)
{
	m_terminate = true;
	m_exitCode = exitCode;
	PostQuitMessage(exitCode);
}

int32_t EventLoopWin32::getExitCode() const
{
	return m_exitCode;
}

int32_t EventLoopWin32::getAsyncKeyState() const
{
	int32_t keyState = KsNone;

	if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0)
		keyState |= KsControl | KsCommand;
	if ((GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0)
		keyState |= KsControl | KsCommand;
	if ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0)
		keyState |= KsMenu;
	if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
		keyState |= KsShift;

	return keyState;
}

bool EventLoopWin32::isKeyDown(VirtualKey vk) const
{
	const uint32_t keyCode = translateToKeyCode(vk);
	if (keyCode)
		return (GetAsyncKeyState(keyCode) & 0x8000) != 0;
	else
		return false;
}

int32_t EventLoopWin32::startTimer(int32_t interval, const std::function< void() >& fn)
{
	// Thread timer; the TIMERPROC is invoked by DispatchMessage, so it fires
	// from any loop pumping this thread's queue, modal loops included.
	const UINT_PTR id = SetTimer(NULL, 0, interval, &timerCallbackProc);
	if (id == 0)
		return -1;

	s_timerCallbacks[id] = fn;

	// Native thread timer ids are small handle-table indices; they must fit
	// the interface's id type.
	T_FATAL_ASSERT(id <= 0x7fffffff);
	return (int32_t)id;
}

void EventLoopWin32::stopTimer(int32_t id)
{
	if (id <= 0)
		return;

	KillTimer(NULL, (UINT_PTR)id);
	s_timerCallbacks.remove((UINT_PTR)id);
}

bool EventLoopWin32::preTranslateMessage(EventSubject* owner, const MSG& msg)
{
	// If some window has capture then we should ignore any global event listeners.
	if (GetCapture() != NULL)
		return false;

	bool consumed = false;
	if (msg.message == WM_KEYDOWN)
	{
		KeyDownEvent keyEvent(owner, translateToVirtualKey(int(msg.wParam)), int(msg.wParam), 0);
		owner->raiseEvent(&keyEvent);
		consumed = keyEvent.consumed();
	}
	else if (msg.message == WM_KEYUP)
	{
		KeyUpEvent keyEvent(owner, translateToVirtualKey(int(msg.wParam)), int(msg.wParam), 0, false);
		owner->raiseEvent(&keyEvent);
		consumed = keyEvent.consumed();
	}
	else if (msg.message == WM_MOUSEMOVE)
	{
		int32_t button = MbtNone;
		if (msg.wParam & MK_LBUTTON)
			button |= MbtLeft;
		if (msg.wParam & MK_MBUTTON)
			button |= MbtMiddle;
		if (msg.wParam & MK_RBUTTON)
			button |= MbtRight;

		POINT pnt = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
		ClientToScreen(msg.hwnd, &pnt);

		Point pt(pnt.x, pnt.y);
		MouseMoveEvent m(owner, button, pt);
		owner->raiseEvent(&m);
	}
	else if (
		msg.message == WM_LBUTTONDOWN || msg.message == WM_MBUTTONDOWN || msg.message == WM_RBUTTONDOWN ||
		msg.message == WM_NCLBUTTONDOWN || msg.message == WM_NCMBUTTONDOWN || msg.message == WM_NCRBUTTONDOWN
	)
	{
		int32_t button = MbtNone;
		switch (msg.message)
		{
		case WM_LBUTTONDOWN:
		case WM_NCLBUTTONDOWN:
			button = MbtLeft;
			break;
		case WM_MBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
			button = MbtMiddle;
			break;
		case WM_RBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
			button = MbtRight;
			break;
		}

		POINT pnt = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
		ClientToScreen(msg.hwnd, &pnt);

		Point pt(pnt.x, pnt.y);
		MouseButtonDownEvent m(owner, button, pt);
		owner->raiseEvent(&m);
	}
	else if (
		msg.message == WM_LBUTTONUP || msg.message == WM_MBUTTONUP || msg.message == WM_RBUTTONUP ||
		msg.message == WM_NCLBUTTONUP || msg.message == WM_NCMBUTTONUP || msg.message == WM_NCRBUTTONUP 
	)
	{
		int32_t button = MbtNone;
		switch (msg.message)
		{
		case WM_LBUTTONUP:
		case WM_NCLBUTTONUP:
			button = MbtLeft;
			break;
		case WM_MBUTTONUP:
		case WM_NCMBUTTONUP:
			button = MbtMiddle;
			break;
		case WM_RBUTTONUP:
		case WM_NCRBUTTONUP:
			button = MbtRight;
			break;
		}

		POINT pnt = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
		ClientToScreen(msg.hwnd, &pnt);

		Point pt(pnt.x, pnt.y);
		MouseButtonUpEvent m(owner, button, pt);
		owner->raiseEvent(&m);
	}
	return consumed;
}

}
