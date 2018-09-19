#include <X11/Xutil.h>
#include "Core/Timer/Timer.h"
#include "Ui/Dialog.h"
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

	// Make dialog on top of parent.
	if (parent != nullptr)
	{
		Window parentWindow = (Window)parent->getInternalHandle();
		XSetTransientForHint(m_display, window, parentWindow);
	}

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

	// Center dialog on parent or desktop.
	Window parentWindow = DefaultRootWindow(m_display);
	Window root;
	int px, py;
	unsigned int pwidth, pheight;
	unsigned int pborder, pdepth;

	if (parent != nullptr && style & Dialog::WsCenterDesktop == 0)
		parentWindow = (Window)parent->getInternalHandle();

	XGetGeometry(
		m_display,
		parentWindow,
		&root,
		&px, &py,
		&pwidth, 
		&pheight,
		&pborder,
		&pdepth
	);

	Rect rc(Point(px + (pwidth - width) / 2, py + (pheight - height) / 2), Size(width, height));

	return WidgetX11Impl< IDialog >::create(nullptr, style, window, rc, false);
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

void DialogX11::endModal(int result)
{
	m_result = result;
	m_modal = false;
}

void DialogX11::setMinSize(const Size& minSize)
{
	XSizeHints sh;
	sh.flags = PMinSize;
	sh.min_width = minSize.cx;
	sh.min_height = minSize.cy;
	XSetWMNormalHints(m_display, m_window, &sh);
}

	}
}
