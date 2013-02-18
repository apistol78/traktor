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

    //XSelectInput(m_display, m_window, ExposureMask | KeyPressMask | KeyReleaseMask);
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

	// Find display mode index.
	int index = -1;
	int sizes = 0;
	XRRScreenSize* xrrss = XRRSizes(m_display, 0, &sizes);

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
		return;
	}

	log::info << L"Using display mode index " << index << Endl;

	// Set display mode.
	XRRScreenConfiguration* xrrc = XRRGetScreenInfo(m_display, RootWindow(m_display, m_screen));
	if (!xrrc)
	{
		log::error << L"XRRGetScreenInfo returned null" << Endl;
		return;
	}

	// Remember original configuration.
	if (!m_originalConfig)
	{
		m_originalConfig = XRRGetScreenInfo(m_display, RootWindow(m_display, m_screen));
		m_originalSizeIndex = XRRConfigCurrentConfiguration(m_originalConfig, &m_originalRotation);
		m_originalRate = XRRConfigCurrentRate(m_originalConfig);
	}

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

	XMoveResizeWindow(m_display, m_window, 0, 0, width, height);
	XSendEvent(m_display, RootWindow(m_display, m_screen), False, SubstructureRedirectMask | SubstructureNotifyMask, &evt);
	XFlush(m_display);
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

	XResizeWindow(m_display, m_window, width, height);
	XSendEvent(m_display, RootWindow(m_display, m_screen), False, SubstructureRedirectMask | SubstructureNotifyMask, &evt);
	XFlush(m_display);
}

void Window::showCursor()
{
	XUndefineCursor(m_display, DefaultRootWindow(m_display));
}

void Window::hideCursor()
{
	static const char c_invisibleCursor[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	XColor black; std::memset(&black, 0, sizeof(black));
	Pixmap bitmapInvisibleCursor = XCreateBitmapFromData(m_display, DefaultRootWindow(m_display), c_invisibleCursor, 8, 8);
	Cursor invisibleCursor = XCreatePixmapCursor(m_display, bitmapInvisibleCursor, bitmapInvisibleCursor, &black, &black, 0, 0);
	XDefineCursor(m_display, DefaultRootWindow(m_display), invisibleCursor);
	XFreeCursor(m_display, invisibleCursor);
}

void Window::show()
{
    XMapWindow(m_display, m_window);
}

bool Window::update(RenderEvent& outEvent)
{
	/*
    XEvent evt;
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
