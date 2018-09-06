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

	Drawable window = XCreateSimpleWindow(
		m_display,
		DefaultRootWindow(m_display),
        0,
		0,
		width,
		height,
		0,
		0,
		0
	);
	
	return WidgetX11Impl< IDialog >::create(parent, window, width, height, false);
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
		tv.tv_usec = 100 * 1000;
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
			Timers::getInstance().update();
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
