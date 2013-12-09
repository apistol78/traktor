#include <cstring>
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
,	m_screen(0)
,	m_width(0)
,	m_height(0)
,	m_fullScreen(false)
,	m_originalConfig(0)
,	m_originalSizeIndex(-1)
,	m_originalRate(0)
,	m_originalRotation(RR_Rotate_0)
{
	m_screen = DefaultScreen(m_display);
}

Window::~Window()
{
	// Set default resolution.
	if (m_originalConfig)
	{
		XRRSetScreenConfigAndRate(
			m_display,
			m_originalConfig,
			RootWindow(m_display, m_screen),
			m_originalSizeIndex,
			m_originalRotation,
			m_originalRate,
			CurrentTime
		);
		XRRFreeScreenConfigInfo(m_originalConfig);
		m_originalConfig = 0;
	}
}

bool Window::create(int32_t width, int32_t height)
{
	m_width = width;
	m_height = height;

    m_window = XCreateSimpleWindow(
        m_display,
        RootWindow(m_display, m_screen),
        10,
        10,
        width,
        height,
        1,
        BlackPixel(m_display, m_screen),
        WhitePixel(m_display, m_screen)
    );

    XSelectInput(
		m_display,
		m_window,
		StructureNotifyMask
	);
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
	Atom wmState = XInternAtom(m_display, "_NET_WM_STATE", False);
	Atom fullScreen = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", False);

	// Get screen configuration.
	XRRScreenConfiguration* xrrc = XRRGetScreenInfo(m_display, RootWindow(m_display, m_screen));
	if (!xrrc)
	{
		log::error << L"XRRGetScreenInfo returned null" << Endl;
		return;
	}

	// Get available display sizes.
	int sizes = 0;
	XRRScreenSize* xrrss = XRRConfigSizes(xrrc, &sizes);

	// Find display mode index.
	int index = -1;
	for (int i = 0; i < sizes; ++i)
	{
		if (xrrss[i].width == width && xrrss[i].height == height)
		{
			index = i;
			break;
		}
	}

	if (index < 0)
	{
		log::error << L"Unable to find matching display mode" << Endl;
		XRRFreeScreenConfigInfo(xrrc);
		return;
	}

	log::info << L"Using display mode index " << index << Endl;

	// Remember original configuration.
	if (!m_originalConfig)
	{
		m_originalConfig = XRRGetScreenInfo(m_display, RootWindow(m_display, m_screen));
		m_originalSizeIndex = XRRConfigCurrentConfiguration(m_originalConfig, &m_originalRotation);
		m_originalRate = XRRConfigCurrentRate(m_originalConfig);
	}

	m_width = width;
	m_height = height;

	// Set new configuration.
	XRRSetScreenConfig(
		m_display,
		xrrc,
		RootWindow(m_display, m_screen),
		index,
		m_originalRotation,
		CurrentTime
	);

	XRRFreeScreenConfigInfo(xrrc);

	XSetWindowAttributes attr;
	attr.override_redirect = True;
	XChangeWindowAttributes(m_display, m_window, CWOverrideRedirect, &attr);

	// Set window in WM fullscreen mode.
	XEvent evt;
	std::memset(&evt, 0, sizeof(evt));
	evt.type = ClientMessage;
	evt.xclient.window = m_window;
	evt.xclient.message_type = wmState;
	evt.xclient.format = 32;
	evt.xclient.data.l[0] = 1;
	evt.xclient.data.l[1] = fullScreen;
	evt.xclient.data.l[2] = 0;

	XSendEvent(
		m_display,
		RootWindow(m_display, m_screen),
		False,
		SubstructureNotifyMask,
		&evt
	);

	XMoveResizeWindow(m_display, m_window, 0, 0, width, height);
	XFlush(m_display);

	m_fullScreen = true;
}

void Window::setWindowedStyle(int32_t width, int32_t height)
{
	Atom wmState = XInternAtom(m_display, "_NET_WM_STATE", False);
	Atom fullScreen = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", False);

	// Set default resolution.
	if (m_originalConfig)
	{
		XRRSetScreenConfigAndRate(
			m_display,
			m_originalConfig,
			RootWindow(m_display, m_screen),
			m_originalSizeIndex,
			m_originalRotation,
			m_originalRate,
			CurrentTime
		);
		XRRFreeScreenConfigInfo(m_originalConfig);
		m_originalConfig = 0;
	}

	if (m_width != width || m_height != height)
	{
        m_width = width;
        m_height = height;

        XSetWindowAttributes attr;
        attr.override_redirect = False;
        XChangeWindowAttributes(m_display, m_window, CWOverrideRedirect, &attr);

        // Remove fullscreen WM state from window.
        XEvent evt;
        std::memset(&evt, 0, sizeof(evt));
        evt.type = ClientMessage;
        evt.xclient.window = m_window;
        evt.xclient.message_type = wmState;
        evt.xclient.format = 32;
        evt.xclient.data.l[0] = 0;
        evt.xclient.data.l[1] = fullScreen;
        evt.xclient.data.l[2] = 0;

        XSendEvent(
            m_display,
            RootWindow(m_display, m_screen),
            False,
            SubstructureNotifyMask,
            &evt
        );

        XResizeWindow(m_display, m_window, width, height);
        XFlush(m_display);

        m_fullScreen = false;
    }
}

void Window::showCursor()
{
#if !defined(_DEBUG)
	Cursor cursor;
	cursor = XCreateFontCursor(m_display, XC_left_ptr);
	XDefineCursor(m_display, DefaultRootWindow(m_display), cursor);
	XFreeCursor(m_display, cursor);
#endif
}

void Window::hideCursor()
{
#if !defined(_DEBUG)
	static const char c_invisibleCursor[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	XColor black; std::memset(&black, 0, sizeof(black));
	Pixmap bitmapInvisibleCursor = XCreateBitmapFromData(m_display, DefaultRootWindow(m_display), c_invisibleCursor, 8, 8);
	Cursor invisibleCursor = XCreatePixmapCursor(m_display, bitmapInvisibleCursor, bitmapInvisibleCursor, &black, &black, 0, 0);
	XDefineCursor(m_display, DefaultRootWindow(m_display), invisibleCursor);
	XFreeCursor(m_display, invisibleCursor);
#endif
}

void Window::show()
{
    XMapWindow(m_display, m_window);
}

void Window::center()
{
    int32_t centerX = (XDisplayWidth(m_display, m_screen) - m_width) / 2;
    int32_t centerY = (XDisplayHeight(m_display, m_screen)- m_height) / 2;
    XMoveWindow(m_display, m_window, centerX, centerY);
    XFlush(m_display);
}

bool Window::update(RenderEvent& outEvent)
{
    XEvent evt;
    if (XCheckWindowEvent(m_display, m_window, StructureNotifyMask, &evt))
    {
    	if (
			evt.type == ConfigureNotify &&
			(
				evt.xconfigure.width != m_width ||
				evt.xconfigure.height != m_height
			)
		)
    	{
			if (!m_fullScreen)
			{
				outEvent.type = ReResize;
				outEvent.resize.width = evt.xconfigure.width;
				outEvent.resize.height = evt.xconfigure.height;
				return true;
			}
    	}
    }
    return false;
}

    }
}
