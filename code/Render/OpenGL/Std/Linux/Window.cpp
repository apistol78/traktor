#include <cstring>
#include <unistd.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/OpenGL/Std/Linux/Window.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const int32_t _NET_WM_STATE_REMOVE = 0;
const int32_t _NET_WM_STATE_ADD = 1;
const int32_t _NET_WM_STATE_TOGGLE = 2;

void setProperty(::Display* display, int32_t screen, ::Window window, const char* propertyName, int32_t value)
{
	Atom wmState = XInternAtom(display, "_NET_WM_STATE", False);
	Atom property = XInternAtom(display, propertyName, False);

	XEvent evt;
	std::memset(&evt, 0, sizeof(evt));
	evt.type = ClientMessage;
	evt.xclient.window = window;
	evt.xclient.message_type = wmState;
	evt.xclient.format = 32;
	evt.xclient.data.l[0] = value;
	evt.xclient.data.l[1] = property;
	evt.xclient.data.l[2] = 0;

	XSendEvent(
		display,
		RootWindow(display, screen),
		False,
		SubstructureNotifyMask,
		&evt
	);
}

void setProperty(::Display* display, int32_t screen, ::Window window, const char* propertyName1, const char* propertyName2, int32_t value)
{
	Atom wmState = XInternAtom(display, "_NET_WM_STATE", False);
	Atom property1 = XInternAtom(display, propertyName1, False);
	Atom property2 = XInternAtom(display, propertyName2, False);

	XEvent evt;
	std::memset(&evt, 0, sizeof(evt));
	evt.type = ClientMessage;
	evt.xclient.window = window;
	evt.xclient.message_type = wmState;
	evt.xclient.format = 32;
	evt.xclient.data.l[0] = value;
	evt.xclient.data.l[1] = property1;
	evt.xclient.data.l[2] = property2;
	evt.xclient.data.l[3] = 0;

	XSendEvent(
		display,
		RootWindow(display, screen),
		False,
		SubstructureNotifyMask,
		&evt
	);
}

void discardX11Events(::Display* display, ::Window window, uint32_t mask)
{
	XEvent evt;
	XFlush(display);
	while (XCheckWindowEvent(display, window, mask, &evt))
		;
}

		}

Window::Window(::Display* display)
:	m_display(display)
,	m_window(None)
,	m_screen(0)
,	m_width(0)
,	m_height(0)
,	m_fullScreen(false)
,	m_active(true)
,	m_cursorShow(true)
,	m_cursorShown(true)
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

	// Disable WM compositor running on our window; this will save performance
	// by reducing offscreen copies.
	const long _NET_WM_BYPASS_COMPOSITOR_HINT_ON = 1;
	Atom wmBypassCompositor = XInternAtom(m_display, "_NET_WM_BYPASS_COMPOSITOR", False);
    XChangeProperty(m_display, m_window, wmBypassCompositor, XA_CARDINAL, 32, PropModeReplace, (uint8_t*)&_NET_WM_BYPASS_COMPOSITOR_HINT_ON, 1);

    // Register event masks.
	XSelectInput(
		m_display,
		m_window,
		ExposureMask | StructureNotifyMask | FocusChangeMask
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

	if (!m_fullScreen)
	{
		// Remove WM borders; ie WM control.
		XSetWindowAttributes attr;
		attr.override_redirect = True;
		XChangeWindowAttributes(m_display, m_window, CWOverrideRedirect, &attr);
		XFlush(m_display);

		// Set window in WM fullscreen mode.
		setProperty(m_display, m_screen, m_window, "_NET_WM_STATE_FULLSCREEN", _NET_WM_STATE_ADD);

		// Consume X11 configure notifications from resize.
		discardX11Events(m_display, m_window, ExposureMask | StructureNotifyMask);

		// \hack Wait a bit, WM might be abit nasty.
		usleep(10 * 1000L);
	}

	XMoveResizeWindow(m_display, m_window, 0, 0, width, height);
	XRaiseWindow(m_display, m_window);

	// Consume X11 configure notifications from resize.
	discardX11Events(m_display, m_window, ExposureMask | StructureNotifyMask);

	m_fullScreen = true;
}

void Window::setWindowedStyle(int32_t width, int32_t height)
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

	if (m_fullScreen || m_width != width || m_height != height)
	{
		m_width = width;
		m_height = height;

		if (m_fullScreen)
		{
			// Remove fullscreen WM state from window.
			setProperty(m_display, m_screen, m_window, "_NET_WM_STATE_FULLSCREEN", _NET_WM_STATE_REMOVE);

			// Remove maximized properties.
			setProperty(m_display, m_screen, m_window, "_NET_WM_STATE_MAXIMIZED_VERT", "_NET_WM_STATE_MAXIMIZED_HORZ", _NET_WM_STATE_REMOVE);

			// Consume X11 configure notifications from resize.
			discardX11Events(m_display, m_window, ExposureMask | StructureNotifyMask);

			// \hack Wait a bit, WM might be abit nasty.
			usleep(10 * 1000L);
		}

		// Get window size; see if we actually need to resize window.
		XWindowAttributes attr;
		XGetWindowAttributes(m_display, m_window, &attr);
		if (attr.width != width || attr.height != height)
			XResizeWindow(m_display, m_window, width, height);
		XRaiseWindow(m_display, m_window);

		// Re-add WM borders; ie WM control.
		XSetWindowAttributes setAttr;
		setAttr.override_redirect = False;
		XChangeWindowAttributes(m_display, m_window, CWOverrideRedirect, &setAttr);

		// Consume X11 configure notifications from resize.
		discardX11Events(m_display, m_window, ExposureMask | StructureNotifyMask);

		m_fullScreen = false;
	}

	// Setup close response from WM.
	Atom wmDeleteWindow = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(m_display, m_window, &wmDeleteWindow, 1);
}

void Window::showCursor()
{
	m_cursorShow = true;
}

void Window::hideCursor()
{
	m_cursorShow = false;
}

void Window::show()
{
	XMapRaised(m_display, m_window);
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

	// First check for explicit Client message; cannot use masked function.
	if (XCheckTypedWindowEvent(m_display, m_window, ClientMessage, &evt))
	{
		Atom wmDeleteWindow = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
		if ((Atom)evt.xclient.data.l[0] == wmDeleteWindow)
		{
			log::info << L"Window closed from WM" << Endl;
			outEvent.type = ReClose;
			return true;
		}
	}

	// Then check for other events we're interested in.
	if (XCheckWindowEvent(m_display, m_window, StructureNotifyMask | FocusChangeMask, &evt))
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
		else if (evt.type == FocusIn)
			m_active = true;
		else if (evt.type == FocusOut)
			m_active = false;
	}

	// Handle mouse cursor show/hide; need to show cursor if window is not active.
	if (m_active && m_cursorShow != m_cursorShown)
	{
		if (m_cursorShow)
		{
			Cursor cursor;
			cursor = XCreateFontCursor(m_display, XC_left_ptr);
			XDefineCursor(m_display, m_window, cursor);
			XFreeCursor(m_display, cursor);
			XFlush(m_display);
		}
		else
		{
			static const char c_invisibleCursor[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
			XColor black; std::memset(&black, 0, sizeof(black));
			Pixmap bitmapInvisibleCursor = XCreateBitmapFromData(m_display, m_window, c_invisibleCursor, 8, 8);
			Cursor invisibleCursor = XCreatePixmapCursor(m_display, bitmapInvisibleCursor, bitmapInvisibleCursor, &black, &black, 0, 0);
			XDefineCursor(m_display, m_window, invisibleCursor);
			XFreeCursor(m_display, invisibleCursor);
			XFlush(m_display);
		}
		m_cursorShown = m_cursorShow;
	}
	else if (!m_active && !m_cursorShown)
	{
		Cursor cursor;
		cursor = XCreateFontCursor(m_display, XC_left_ptr);
		XDefineCursor(m_display, m_window, cursor);
		XFreeCursor(m_display, cursor);
		XFlush(m_display);
		m_cursorShown = true;
	}

	return false;
}

	}
}

