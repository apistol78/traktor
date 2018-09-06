#include "Core/Log/Log.h"
#include "Ui/X11/Assoc.h"
#include "Ui/X11/EventLoopX11.h"
#include "Ui/X11/Timers.h"

namespace traktor
{
	namespace ui
	{

EventLoopX11::EventLoopX11(Display* display, int32_t screen)
:	m_display(display)
,	m_screen(screen)
,	m_terminated(false)
,	m_exitCode(0)
{
}

EventLoopX11::~EventLoopX11()
{
}

void EventLoopX11::destroy()
{
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
	Assoc::getInstance().dispatch(e);

	return !m_terminated;
}

int32_t EventLoopX11::execute(EventSubject* owner)
{
	XEvent e;

	int fd = ConnectionNumber(m_display);

	while (!m_terminated)
	{
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        struct timeval tv;
        tv.tv_usec = 100 * 1000;
        tv.tv_sec = 0;

        int nr = select(fd + 1, &fds, NULL, NULL, &tv);
        if (nr > 0)
		{
			while (XPending(m_display))
			{
				XNextEvent(m_display, &e);
				Assoc::getInstance().dispatch(e);
			}
		}
		else
		{
			Timers::getInstance().update();
		}
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
	return 0;
}

bool EventLoopX11::isKeyDown(VirtualKey vk) const
{
	return false;
}

Size EventLoopX11::getDesktopSize() const
{
	return Size(1280, 720);
}

	}
}

