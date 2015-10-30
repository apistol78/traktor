#include "Ui/Enums.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/FocusEvent.h"
#include "Ui/Events/IdleEvent.h"
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

namespace traktor
{
	namespace ui
	{

EventLoopWin32::EventLoopWin32()
:	m_exitCode(0)
,	m_terminate(false)
{
#if defined(T_STATIC)
	g_hInstance = (HINSTANCE)GetModuleHandle(NULL);
#endif

	OleInitialize(NULL);

	INITCOMMONCONTROLSEX icc;
	std::memset(&icc, 0, sizeof(icc));
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES;
	InitCommonControlsEx(&icc);

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

bool EventLoopWin32::process(EventSubject* owner)
{
	if (m_terminate)
		return false;

	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		bool dispatch = !preTranslateMessage(owner, msg);
		if (dispatch)
		{
			HWND hwndFocus = GetFocus();
			HWND hwndTop = hwndFocus;

			for (; GetParent(hwndTop) != NULL; hwndTop = GetParent(hwndTop))
				;

			BOOL handled = FALSE;
			if (hwndTop != NULL)
				handled = IsDialogMessage(hwndTop, &msg);

			if (!handled)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);

				if (hwndFocus != GetFocus())
				{
					FocusEvent focusEvent(owner, true);
					owner->raiseEvent(&focusEvent);
				}
			}
		}
	}

	return true;
}

int EventLoopWin32::execute(EventSubject* owner)
{
	bool m_idle = false;
	MSG msg;

	while (!m_terminate)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			bool dispatch = !preTranslateMessage(owner, msg);
			if (dispatch)
			{
				HWND hwndFocus = GetFocus();
				HWND hwndTop = hwndFocus;

				for (; hwndTop != NULL && GetParent(hwndTop) != NULL; hwndTop = GetParent(hwndTop))
					;

				BOOL handled = FALSE;
				if (hwndTop != NULL)
					handled = IsDialogMessage(hwndTop, &msg);

				if (!handled)
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);

					if (hwndFocus != GetFocus())
					{
						FocusEvent focusEvent(owner, true);
						owner->raiseEvent(&focusEvent);
					}
				}
			}

			m_idle = false;
			continue;
		}

		if (!m_idle)
		{
			IdleEvent idleEvent(owner);
			owner->raiseEvent(&idleEvent);
			if (!idleEvent.requestedMore())
				WaitMessage();
		}
	}

	return m_exitCode;
}

void EventLoopWin32::exit(int exitCode)
{
	m_terminate = true;
	m_exitCode = exitCode;
	PostQuitMessage(exitCode);
}

int EventLoopWin32::getExitCode() const
{
	return m_exitCode;
}

int EventLoopWin32::getAsyncKeyState() const
{
	int keyState = KsNone;

	if (GetAsyncKeyState(VK_CONTROL))
		keyState |= KsControl | KsCommand;
	if (GetAsyncKeyState(VK_MENU))
		keyState |= KsMenu;
	if (GetAsyncKeyState(VK_SHIFT))
		keyState |= KsShift;

	return keyState;
}

bool EventLoopWin32::preTranslateMessage(EventSubject* owner, const MSG& msg)
{
	bool consumed = false;
	if (msg.message == WM_KEYDOWN)
	{
		KeyDownEvent keyEvent(owner, translateKeyCode(int(msg.wParam)), int(msg.wParam), 0);
		owner->raiseEvent(&keyEvent);
		consumed = keyEvent.consumed();
	}
	else if (msg.message == WM_KEYUP)
	{
		KeyUpEvent keyEvent(owner, translateKeyCode(int(msg.wParam)), int(msg.wParam), 0);
		owner->raiseEvent(&keyEvent);
		consumed = keyEvent.consumed();
	}
	return consumed;
}

	}
}
