#include <X11/Xatom.h>
#include "Ui/X11/ToolFormX11.h"

namespace traktor
{
	namespace ui
	{

ToolFormX11::ToolFormX11(EventSubject* owner, Display* display, int32_t screen, XIM xim)
:	WidgetX11Impl< IToolForm >(owner, display, screen, xim)
,	m_result(0)
,	m_modal(false)
{
}

bool ToolFormX11::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	const int32_t c_minWidth = 16;
	const int32_t c_minHeight = 16;

	width = std::max< int32_t >(width, c_minWidth);
	height = std::max< int32_t >(height, c_minHeight);

	Window window = XCreateWindow(
		m_display,
		DefaultRootWindow(m_display),
		0,
		0,
		width,
		height,
		0,
		0,
		InputOutput,
		CopyFromParent,
		0,
		nullptr
	);

	// Change style of window, no WM chrome.
    Atom type = XInternAtom(m_display,"_NET_WM_WINDOW_TYPE", False);
    Atom value = XInternAtom(m_display,"_NET_WM_WINDOW_TYPE_DOCK", False);
    XChangeProperty(m_display, window, type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&value), 1);

	// Make tool form on top of parent.
	if (parent != nullptr)
	{
		Window parentWindow = (Window)parent->getInternalHandle();
		XSetTransientForHint(m_display, window, parentWindow);
	}

	return WidgetX11Impl< IToolForm >::create(nullptr, style, window, Rect(0, 0, width, height), false);
}

void ToolFormX11::destroy()
{
	WidgetX11Impl< IToolForm >::destroy();
}

int ToolFormX11::showModal()
{
	setWmProperty("_NET_WM_STATE_MODAL", _NET_WM_STATE_ADD);
	setVisible(true);

	XFlush(m_display);

	int fd = ConnectionNumber(m_display);
	XEvent e;

	Timer timer;
	timer.start();

	for (m_modal = true; m_modal; )
	{
        int nr = 0;
		if (!XPending(m_display))
		{
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			struct timeval tv;
			tv.tv_usec = 1 * 1000;
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
				Assoc::getInstance().dispatch(m_display, e);
			}
		}

		double dt = timer.getDeltaTime();
		Timers::getInstance().update(dt);
	}

	setVisible(false);
	setWmProperty("_NET_WM_STATE_MODAL", _NET_WM_STATE_REMOVE);

	return m_result;
}

void ToolFormX11::endModal(int result)
{
	m_result = result;
	m_modal = false;
}

	}
}

