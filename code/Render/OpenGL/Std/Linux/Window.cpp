#include "Render/OpenGL/Std/Linux/Window.h"

namespace traktor
{
    namespace render
    {

Window::Window()
:   m_display(0)
,   m_window(None)
{
}

Window::~Window()
{
    if (m_display)
        XCloseDisplay(m_display);
}

bool Window::create()
{
    m_display = XOpenDisplay(0);
    if (!m_display)
        return false;

    int screen = DefaultScreen(m_display);

    m_window = XCreateSimpleWindow(
        m_display,
        RootWindow(m_display, screen),
        10,
        10,
        200,
        200,
        1,
        BlackPixel(m_display, screen),
        WhitePixel(m_display, screen)
    );

    XSelectInput(m_display, m_window, ExposureMask | KeyPressMask);
    XMapWindow(m_display, m_window);

    return true;
}

bool Window::update()
{
    XEvent evt;
    XNextEvent(m_display, &evt);
    return true;
}

    }
}
