#include "Ui/Win32/EventLoopWin32.h"
#if defined(T_USE_GDI_PLUS)
#include "Ui/Win32/CanvasGdiPlusWin32.h"
#endif
#include "Ui/Enums.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/IdleEvent.h"

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

#if defined(WINCE)
	SHInitExtraControls();
#endif

	INITCOMMONCONTROLSEX icc;
	memset(&icc, 0, sizeof(icc));
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES;
	InitCommonControlsEx(&icc);

#if defined(T_USE_GDI_PLUS)
	CanvasGdiPlusWin32::startup();
#endif

	Window::registerDefaultClass();
}

EventLoopWin32::~EventLoopWin32()
{
	Window::unregisterDefaultClass();

#if defined(T_USE_GDI_PLUS)
	CanvasGdiPlusWin32::shutdown();
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
			TranslateMessage(&msg);
			DispatchMessage(&msg);
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
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			m_idle = false;
			continue;
		}
		if (!m_idle)
		{
			IdleEvent idleEvent(owner);
			owner->raiseEvent(EiIdle, &idleEvent);
#if !defined(WINCE)
			if (!idleEvent.requestedMore())
				WaitMessage();
#endif
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
		keyState |= KsControl;
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
		KeyEvent k(owner, 0, int(msg.wParam));
		owner->raiseEvent(EiKeyDown, &k);
		consumed = k.consumed();
	}
	else if (msg.message == WM_KEYUP)
	{
		KeyEvent k(owner, 0, int(msg.wParam));
		owner->raiseEvent(EiKeyUp, &k);
		consumed = k.consumed();
	}
	return consumed;
}

	}
}
