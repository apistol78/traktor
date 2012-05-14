#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/OpenGL/Std/Linux/Window.h"

namespace traktor
{
    namespace render
    {

Window::Window(::Display* display)
:   m_display(display)
,   m_window(None)
{
}

Window::~Window()
{
}

bool Window::create(int32_t width, int32_t height)
{
    int screen = DefaultScreen(m_display);

    m_window = XCreateSimpleWindow(
        m_display,
        RootWindow(m_display, screen),
        10,
        10,
        width,
        height,
        1,
        BlackPixel(m_display, screen),
        WhitePixel(m_display, screen)
    );

    XSelectInput(m_display, m_window, ExposureMask | KeyPressMask | KeyReleaseMask);
    XMapWindow(m_display, m_window);

	T_DEBUG(L"Render window " << int32_t(m_window));
    return true;
}

void Window::setTitle(const wchar_t* title)
{
	std::string cs = wstombs(title);
	const char* csp = cs.c_str();

	XTextProperty tp;
	XStringListToTextProperty((char**)&csp, 1, &tp);

	XSetWMName(m_display, m_window, &tp);
}

void Window::setFullScreenStyle(int32_t width, int32_t height)
{
	/*
	XResizeWindow(m_display, m_window, width, height);
	XFlush(m_display);
	XSync(m_display, True);
	*/
}

void Window::setWindowedStyle(int32_t width, int32_t height)
{
	/*
	XResizeWindow(m_display, m_window, width, height);
	XFlush(m_display);
	XSync(m_display, True);
	*/
}

bool Window::update(RenderEvent& outEvent)
{
    XEvent evt;

	/*
    if (XCheckWindowEvent(m_display, m_window, ResizeRedirectMask, &evt))
    {
    	if (evt.type == ResizeRequest)
    	{
    		outEvent.type = ReResize;
    		outEvent.resize.width = evt.xresizerequest.width;
    		outEvent.resize.height = evt.xresizerequest.height;
    		T_DEBUG(L"Resize event " << outEvent.resize.width << L" x " << outEvent.resize.height);
    		return true;
    	}
    }
    */

    return false;
}

int32_t Window::getWidth() const
{
	XWindowAttributes attr;
	XGetWindowAttributes(m_display, m_window, &attr);
	return attr.width;
}

int32_t Window::getHeight() const
{
	XWindowAttributes attr;
	XGetWindowAttributes(m_display, m_window, &attr);
	return attr.height;
}

    }
}
