#include <X11/Xatom.h>
#include "Core/Timer/Timer.h"
#include "Ui/X11/ToolFormX11.h"

namespace traktor
{
	namespace ui
	{

ToolFormX11::ToolFormX11(Context* context, EventSubject* owner)
:	WidgetX11Impl< IToolForm >(context, owner)
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
		m_context->getDisplay(),
		DefaultRootWindow(m_context->getDisplay()),
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
    Atom type = XInternAtom(m_context->getDisplay(),"_NET_WM_WINDOW_TYPE", False);
    Atom value = XInternAtom(m_context->getDisplay(),"_NET_WM_WINDOW_TYPE_DOCK", False);
    XChangeProperty(m_context->getDisplay(), window, type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&value), 1);

	// Make tool form on top of parent.
	if (parent != nullptr)
	{
		WidgetData* parentData = static_cast< WidgetData* >(parent->getInternalHandle());
		XSetTransientForHint(m_context->getDisplay(), window, parentData->window);
	}

	return WidgetX11Impl< IToolForm >::create(parent, style, window, Rect(0, 0, width, height), false, true);
}

void ToolFormX11::destroy()
{
	T_FATAL_ASSERT (m_modal == false);
	WidgetX11Impl< IToolForm >::destroy();
}

int ToolFormX11::showModal()
{
	setWmProperty("_NET_WM_STATE_MODAL", _NET_WM_STATE_ADD);
	setVisible(true);

	XFlush(m_context->getDisplay());

	m_context->pushModal(&m_data);

	int fd = ConnectionNumber(m_context->getDisplay());
	XEvent e;

	Timer timer;
	timer.start();

	for (m_modal = true; m_modal; )
	{
        int nr = 0;
		if (!XPending(m_context->getDisplay()))
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
			while (XPending(m_context->getDisplay()))
			{
				XNextEvent(m_context->getDisplay(), &e);
				m_context->dispatch(e);
			}
		}

		double dt = timer.getDeltaTime();
		Timers::getInstance().update(dt);
	}

	setVisible(false);
	setWmProperty("_NET_WM_STATE_MODAL", _NET_WM_STATE_REMOVE);

	m_context->popModal();

	return m_result;
}

void ToolFormX11::endModal(int result)
{
	m_result = result;
	m_modal = false;
}

	}
}

