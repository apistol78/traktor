#include <X11/Xutil.h>
#include "Ui/X11/Assoc.h"
#include "Ui/X11/DialogX11.h"
#include "Ui/X11/Timers.h"

namespace traktor
{
	namespace ui
	{

DialogX11::DialogX11(EventSubject* owner, Display* display, int32_t screen)
:	WidgetX11Impl< IDialog >(owner, display, screen)
,	m_result(0)
,	m_modal(false)
{
}

bool DialogX11::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
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
	
	// Notify WM about form title.
	std::string cs = wstombs(text);
	const char* csp = cs.c_str();

	XTextProperty tp;
	XStringListToTextProperty((char**)&csp, 1, &tp);

	XSetWMName(m_display, window, &tp);

	// Register "delete window" window manager message.
	m_atomWmDeleteWindow = XInternAtom(m_display, "WM_DELETE_WINDOW", False);

	Assoc::getInstance().bind(window, ClientMessage, [&](XEvent& xe){
		if ((Atom)xe.xclient.data.l[0] == m_atomWmDeleteWindow)
		{
			CloseEvent closeEvent(m_owner);
			m_owner->raiseEvent(&closeEvent);
			if (!(closeEvent.consumed() && closeEvent.cancelled()))
				endModal(DrCancel);
		}		
	});

	XSetWMProtocols(m_display, window, &m_atomWmDeleteWindow, 1);

	return WidgetX11Impl< IDialog >::create(nullptr, window, Rect(0, 0, width, height), false);
}

void DialogX11::destroy()
{
	WidgetX11Impl< IDialog >::destroy();
}

void DialogX11::setIcon(ISystemBitmap* icon)
{
}

int DialogX11::showModal()
{
	setVisible(true);

	int fd = ConnectionNumber(m_display);
	for (m_modal = true; m_modal; )
	{
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		struct timeval tv;
		tv.tv_usec = 10 * 1000;
		tv.tv_sec = 0;

		int nr = select(fd + 1, &fds, NULL, NULL, &tv);
		if (nr > 0)
		{
			while (XPending(m_display))
			{
				XEvent e;
				XNextEvent(m_display, &e);
				Assoc::getInstance().dispatch(e);
			}
		}
		else
		{
			Timers::getInstance().update(10);
		}
	}

	setVisible(false);
	return m_result;
}

void DialogX11::endModal(int result)
{
	m_result = result;
	m_modal = false;
}

void DialogX11::setMinSize(const Size& minSize)
{
}

	}
}
