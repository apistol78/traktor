#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Timer/Timer.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/IdleEvent.h"
#include "Ui/Events/KeyDownEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/KeyUpEvent.h"
#include "Ui/X11/Assoc.h"
#include "Ui/X11/EventLoopX11.h"
#include "Ui/X11/Timers.h"
#include "Ui/X11/UtilitiesX11.h"

namespace traktor
{
	namespace ui
	{

EventLoopX11::EventLoopX11(Display* display, int32_t screen)
:	m_display(display)
,	m_screen(screen)
,	m_terminated(false)
,	m_exitCode(0)
,	m_keyState(0)
{
	// Open input method.
	XSetLocaleModifiers("");
	if ((m_xim = XOpenIM(m_display, nullptr, nullptr, nullptr)) == 0)
	{
		XSetLocaleModifiers("@im=");
		if ((m_xim = XOpenIM(m_display, nullptr, nullptr, nullptr)) == 0)
		{
			log::error << L"XOpenIM failed." << Endl;
			return;
		}
	}

	// Create input context.
	if ((m_xic = XCreateIC(
		m_xim,
		XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
		nullptr
	)) == 0)
	{
		log::error << L"XCreateIC failed." << Endl;
		return;
	}
}

EventLoopX11::~EventLoopX11()
{
}

void EventLoopX11::destroy()
{
	if (m_xic != 0)
	{
		XDestroyIC(m_xic);
		m_xic = 0;
	}

	if (m_xim != 0)
	{
		XCloseIM(m_xim);
		m_xim = 0;
	}

	delete this;
}

bool EventLoopX11::process(EventSubject* owner)
{
	if (m_terminated)
		return false;

	if (!XPending(m_display))
		return !m_terminated;

	XEvent e;
	XNextEvent(m_display, &e);

	if (!preTranslateEvent(owner, e))
		Assoc::getInstance().dispatch(m_display, e);

	return !m_terminated;
}

int32_t EventLoopX11::execute(EventSubject* owner)
{
	XEvent e;

	int fd = ConnectionNumber(m_display);
	bool idle = true;

	Timer timer;
	timer.start();

	while (!m_terminated)
	{
        int nr = 0;
		if (!XPending(m_display))
		{
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			struct timeval tv;
			tv.tv_usec = idle ? 10 : 1 * 1000;
			tv.tv_sec = 0;

			nr = select(fd + 1, &fds, nullptr, nullptr, &tv);
		}
		else
			nr = 1;

        if (nr > 0)
		{
			while (XPending(m_display))
			{
				XNextEvent(m_display, &e);

				if (!preTranslateEvent(owner, e))
					Assoc::getInstance().dispatch(m_display, e);

				idle = true;
			}
		}
		else
		{
			if (idle)
			{
				IdleEvent idleEvent(owner);
				owner->raiseEvent(&idleEvent);
				if (!idleEvent.requestedMore())
					idle = false;
			}
		}

		double dt = timer.getDeltaTime();
		Timers::getInstance().update(dt);
	}

	return m_exitCode;
}

void EventLoopX11::exit(int32_t exitCode)
{
	m_exitCode = exitCode;
	m_terminated = true;
}

int32_t EventLoopX11::getExitCode() const
{
	return m_exitCode;
}

int32_t EventLoopX11::getAsyncKeyState() const
{
	int32_t keyState = KsNone;

	if (m_keyState & ShiftMask)
		keyState |= KsShift;
	if (m_keyState & ControlMask)
		keyState |= KsControl | KsCommand;
	if (m_keyState & (Mod1Mask | Mod5Mask))
		keyState |= KsMenu;

	return keyState;
}

bool EventLoopX11::isKeyDown(VirtualKey vk) const
{
	return false;
}

Size EventLoopX11::getDesktopSize() const
{
	return Size(1280, 720);
}

bool EventLoopX11::preTranslateEvent(EventSubject* owner, XEvent& e)
{
	bool consumed = false;

	if (e.type == KeyPress)
	{
		m_keyState = e.xkey.state;

		int nkeysyms;
		KeySym* ks = XGetKeyboardMapping(m_display, e.xkey.keycode, 1, &nkeysyms);
		if (ks != nullptr)
		{
			VirtualKey vk = translateToVirtualKey(ks, nkeysyms);
			if (vk != VkNull)
			{
				KeyDownEvent keyDownEvent(owner, vk, e.xkey.keycode, 0);
				owner->raiseEvent(&keyDownEvent);
				consumed |= keyDownEvent.consumed();
			}

			uint8_t str[8] = { 0 };

			Status status = 0;
			const int n = Xutf8LookupString(m_xic, &e.xkey, (char*)str, 8, ks, &status);
			if (n > 0)
			{
				wchar_t wch = 0;
				if (Utf8Encoding().translate(str, n, wch) > 0)
				{
					KeyEvent keyEvent(owner, vk, e.xkey.keycode, wch);
					owner->raiseEvent(&keyEvent);
					consumed |= keyEvent.consumed();
				}
			}

			XFree(ks);
		}
	}
	else if (e.type == KeyRelease)
	{
		m_keyState = e.xkey.state;

		int nkeysyms;
		KeySym* ks = XGetKeyboardMapping(m_display, e.xkey.keycode, 1, &nkeysyms);
		if (ks != nullptr)
		{
			VirtualKey vk = translateToVirtualKey(ks, nkeysyms);
			if (vk != VkNull)
			{
				KeyUpEvent keyUpEvent(owner, vk, e.xkey.keycode, 0);
				owner->raiseEvent(&keyUpEvent);
				consumed |= keyUpEvent.consumed();
			}
			XFree(ks);
		}
	}
	else if (e.type == MotionNotify)
	{
		m_keyState = e.xmotion.state;
	}
	else if (e.type == ButtonPress || e.type == ButtonRelease)
	{
		m_keyState = e.xbutton.state;
	}

	return consumed;
}

	}
}

