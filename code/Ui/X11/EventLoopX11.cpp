/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/IdleEvent.h"
#include "Ui/Events/MouseButtonDownEvent.h"
#include "Ui/Events/MouseButtonUpEvent.h"
#include "Ui/Events/MouseMoveEvent.h"
#include "Ui/Events/KeyDownEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/KeyUpEvent.h"
#include "Ui/X11/Context.h"
#include "Ui/X11/EventLoopX11.h"
#include "Ui/X11/Timers.h"
#include "Ui/X11/UtilitiesX11.h"

namespace traktor::ui
{
	namespace
	{

MouseMoveEvent createMouseMoveEvent(Context* context, EventSubject* owner, XEvent& e)
{
	int32_t button = 0;
	if ((e.xmotion.state & Button1Mask) != 0)
		button = MbtLeft;
	if ((e.xmotion.state & Button2Mask) != 0)
		button = MbtMiddle;
	if ((e.xmotion.state & Button3Mask) != 0)
		button = MbtRight;

	Window dw; int x, y;
	XTranslateCoordinates(
		context->getDisplay(),
		e.xmotion.window,
		DefaultRootWindow(context->getDisplay()),
		e.xmotion.x,
		e.xmotion.y,
		&x,
		&y,
		&dw
	);

	return MouseMoveEvent(
		owner,
		button,
		Point(x, y)
	);
}


	}

EventLoopX11::EventLoopX11(Context* context)
:	m_context(context)
,	m_terminated(false)
,	m_exitCode(0)
,	m_keyState(0)
{
	// Create input context.
	if ((m_xic = XCreateIC(
		m_context->getXIM(),
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
	delete this;
}

bool EventLoopX11::process(EventSubject* owner)
{
	SmallSet< Window > exposeWindows;
	XEvent e;

	if (m_terminated)
		return false;

	// Read all pending events, do not dispatch expose events
	// directly since we synthesize those later to reduce number of redraws.
	while (XPending(m_context->getDisplay()))
	{
		XNextEvent(m_context->getDisplay(), &e);
		if (e.type != Expose)
		{
			if (!preTranslateEvent(owner, e))
				m_context->dispatch(e);
		}
		else
			exposeWindows.insert(e.xexpose.window);
	}

	for (auto window : exposeWindows)
	{
		e.type = Expose;
		e.xexpose.window = window;
		e.xexpose.count = 0;
		m_context->dispatch(e);
	}

	double dt = m_timer.getDeltaTime();
	Timers::getInstance().update(dt);

	return !m_terminated;
}

int32_t EventLoopX11::execute(EventSubject* owner)
{
	SmallSet< Window > exposeWindows;
	XEvent pendingMotion;
	XEvent e;

	pendingMotion.type = 0;

	int fd = ConnectionNumber(m_context->getDisplay());
	bool idle = true;

	while (!m_terminated)
	{
		// Process events; do this even if select doesn't return any waiting
		// due to some strange behaviour of X11 events.
		{
			exposeWindows.reset();

			// Read all pending events, do not dispatch expose events
			// directly since we synthesize those later to reduce number of redraws.
			for (;;)
			{
				if (XPending(m_context->getDisplay()))
				{
					XNextEvent(m_context->getDisplay(), &e);

					if (e.type == MotionNotify)
					{
						if (pendingMotion.type == MotionNotify)
						{
							if (
								pendingMotion.xmotion.window == e.xmotion.window &&
								pendingMotion.xmotion.state == e.xmotion.state
							)
								pendingMotion = e;
							else
							{
								if (!preTranslateEvent(owner, pendingMotion))
									m_context->dispatch(pendingMotion);								
							}
						}
						pendingMotion = e;
					}
					else if (e.type == Expose)
						exposeWindows.insert(e.xexpose.window);
					else
					{
						if (pendingMotion.type == MotionNotify)
						{
							if (!preTranslateEvent(owner, pendingMotion))
								m_context->dispatch(pendingMotion);
							pendingMotion.type = 0;						
						}

						if (!preTranslateEvent(owner, e))
							m_context->dispatch(e);
					}
				}
				else
				{
					fd_set fds;
					FD_ZERO(&fds);
					FD_SET(fd, &fds);

					struct timeval tv;
					tv.tv_usec = 1 * 1000;
					tv.tv_sec = 0;
					select(fd + 1, &fds, nullptr, nullptr, &tv);

					if (!XPending(m_context->getDisplay()))
					{
						idle = true;
						break;
					}
				}
			}

			if (pendingMotion.type == MotionNotify)
			{
				if (!preTranslateEvent(owner, pendingMotion))
					m_context->dispatch(pendingMotion);
				pendingMotion.type = 0;						
			}
			
			for (auto window : exposeWindows)
			{
				e.type = Expose;
				e.xexpose.window = window;
				e.xexpose.count = 0;
				m_context->dispatch(e);
			}
		}

		if (idle)
		{
			IdleEvent idleEvent(owner);
			owner->raiseEvent(&idleEvent);
			if (!idleEvent.requestedMore())
				idle = false;
		}

		const double dt = m_timer.getDeltaTime();
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

bool EventLoopX11::preTranslateEvent(EventSubject* owner, XEvent& e)
{
	bool consumed = false;

	if (m_context->anyGrabbed())
		return false;

	if (e.type == KeyPress)
	{
		m_keyState = e.xkey.state;

		int nkeysyms;
		KeySym* ks = XGetKeyboardMapping(m_context->getDisplay(), e.xkey.keycode, 1, &nkeysyms);
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
		KeySym* ks = XGetKeyboardMapping(m_context->getDisplay(), e.xkey.keycode, 1, &nkeysyms);
		if (ks != nullptr)
		{
			VirtualKey vk = translateToVirtualKey(ks, nkeysyms);
			if (vk != VkNull)
			{
				bool repeat = false;
				if (XPending(m_context->getDisplay()))
				{
					XEvent nextEvent;
					XPeekEvent(m_context->getDisplay(), &nextEvent);
					repeat = (nextEvent.type == KeyPress && nextEvent.xkey.time == e.xkey.time && nextEvent.xkey.keycode == e.xkey.keycode);
				}

				KeyUpEvent keyUpEvent(owner, vk, e.xkey.keycode, 0, repeat);
				owner->raiseEvent(&keyUpEvent);
				consumed |= keyUpEvent.consumed();
			}
			XFree(ks);
		}
	}
	else if (e.type == MotionNotify)
	{
		m_keyState = e.xmotion.state;

		int32_t button = 0;
		if ((e.xmotion.state & Button1Mask) != 0)
			button = MbtLeft;
		if ((e.xmotion.state & Button2Mask) != 0)
			button = MbtMiddle;
		if ((e.xmotion.state & Button3Mask) != 0)
			button = MbtRight;

		Window dw; int x, y;
		XTranslateCoordinates(
			m_context->getDisplay(),
			e.xmotion.window,
			DefaultRootWindow(m_context->getDisplay()),
			e.xmotion.x,
			e.xmotion.y,
			&x,
			&y,
			&dw
		);

		MouseMoveEvent mouseMoveEvent(
			owner,
			button,
			Point(x, y)
		);
		owner->raiseEvent(&mouseMoveEvent);
	}
	else if (e.type == ButtonPress || e.type == ButtonRelease)
	{
		m_keyState = e.xbutton.state;

		int32_t button = 0;
		if ((e.xbutton.state & Button1Mask) != 0)
			button = MbtLeft;
		if ((e.xbutton.state & Button2Mask) != 0)
			button = MbtMiddle;
		if ((e.xbutton.state & Button3Mask) != 0)
			button = MbtRight;

		Window dw; int x, y;
		XTranslateCoordinates(
			m_context->getDisplay(),
			e.xbutton.window,
			DefaultRootWindow(m_context->getDisplay()),
			e.xbutton.x,
			e.xbutton.y,
			&x,
			&y,
			&dw
		);

		if (e.type == ButtonPress)
		{
			MouseButtonDownEvent mouseButtonDownEvent(
				owner,
				button,
				Point(x, y)
			);
			owner->raiseEvent(&mouseButtonDownEvent);
		}
		else
		{
			MouseButtonUpEvent mouseButtonUpEvent(
				owner,
				button,
				Point(x, y)
			);
			owner->raiseEvent(&mouseButtonUpEvent);
		}
	}

	return consumed;
}

}
