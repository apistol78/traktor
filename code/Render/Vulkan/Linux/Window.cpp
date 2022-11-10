/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <unistd.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Containers/AlignedVector.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Render/Vulkan/Linux/Window.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const int32_t _NET_WM_STATE_REMOVE = 0;
const int32_t _NET_WM_STATE_ADD = 1;
const int32_t _NET_WM_STATE_TOGGLE = 2;

		}

Window::Window()
:	m_display(nullptr)
,	m_window(None)
,	m_screen(0)
,	m_width(0)
,	m_height(0)
,	m_fullScreen(false)
,	m_active(true)
,	m_cursorShow(true)
,	m_cursorShown(true)
{
}

Window::~Window()
{
}

bool Window::create(int32_t width, int32_t height)
{
	if ((m_display = XOpenDisplay(0)) == nullptr)
	{
		log::error << L"Unable to create Vulkan renderer; Failed to open X display" << Endl;
		return false;
	}

	// Initialize X11 protocol atoms.
	m_atomWmBypassCompositor = XInternAtom(m_display, "_NET_WM_BYPASS_COMPOSITOR", False);
	m_atomWmState = XInternAtom(m_display, "_NET_WM_STATE", False);
	m_atomWmStateFullscreen = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", False);
	m_atomWmStateMaximizedVert = XInternAtom(m_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	m_atomWmStateMaximizedHorz= XInternAtom(m_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	m_atomWmStateAbove = XInternAtom(m_display, "_NET_WM_STATE_ABOVE", False);
	m_atomWmDeleteWindow = XInternAtom(m_display, "WM_DELETE_WINDOW", False);

	m_screen = DefaultScreen(m_display);
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
    XChangeProperty(
		m_display,
		m_window,
		m_atomWmBypassCompositor,
		XA_CARDINAL,
		32,
		PropModeReplace,
		(uint8_t*)&_NET_WM_BYPASS_COMPOSITOR_HINT_ON,
		1
	);

    // Register event masks.
	XSelectInput(
		m_display,
		m_window,
		ExposureMask | StructureNotifyMask | FocusChangeMask
	);

	// Load icon; should be in working directory.
	Ref< drawing::Image > icon = drawing::Image::load(L"Default.png");
	if (icon != nullptr)
	{
		icon->convert(drawing::PixelFormat::getA8R8G8B8());

		const int32_t szs[] = { 16, 32, 64, 128, 256, 0 };
		AlignedVector< unsigned long > data;

		for (const int32_t* sz = szs; *sz != 0; ++sz)
		{
			int32_t w = *sz;
			int32_t h = *sz;

			drawing::ScaleFilter sf(
				w,
				h,
				drawing::ScaleFilter::MnAverage,
				drawing::ScaleFilter::MgLinear
			);

			Ref< drawing::Image > iconMip = icon->clone();
			iconMip->apply(&sf);

			data.push_back(w);
			data.push_back(h);

			uint32_t o = data.size();
			data.resize(o + w * h);

			const uint32_t* src = static_cast< const uint32_t* >(iconMip->getData());
			for (uint32_t i = 0; i < w * h; ++i)
				data[o + i] = (unsigned long)src[i];
		}

		XChangeProperty(
			m_display,
			m_window,
			XInternAtom(m_display, "_NET_WM_ICON", False),
			XA_CARDINAL, 32,
			PropModeReplace,
			(unsigned char*)data.ptr(),
			data.size()
		);
	}

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

void Window::setFullScreenStyle(int32_t /*width*/, int32_t /*height*/)
{
	int32_t width = XDisplayWidth(m_display, m_screen);
	int32_t height = XDisplayHeight(m_display, m_screen);

	if (!m_fullScreen)
	{
		m_fullScreen = true;

		// Set WM properties for fullscreen.
		setWmProperty(m_atomWmStateFullscreen, _NET_WM_STATE_ADD);

		// Remove WM borders; ie WM control.
		// XSetWindowAttributes attr;
		// attr.override_redirect = True;
		// XChangeWindowAttributes(m_display, m_window, CWOverrideRedirect, &attr);
	}

	XMoveResizeWindow(m_display, m_window, 0, 0, width, height);
	XFlush(m_display);

	m_width = width;
	m_height = height;
	m_fullScreen = true;
}

void Window::setWindowedStyle(int32_t width, int32_t height)
{
	if (m_fullScreen || m_width != width || m_height != height)
	{
		// Re-add WM borders; ie WM control.
		XSetWindowAttributes setAttr;
		setAttr.override_redirect = False;
		XChangeWindowAttributes(m_display, m_window, CWOverrideRedirect, &setAttr);

		if (m_fullScreen)
		{
			// Remove fullscreen WM state from window.
			setWmProperty(m_atomWmStateFullscreen, _NET_WM_STATE_REMOVE);
		}

		if (m_width != width || m_height != height)
			XResizeWindow(m_display, m_window, width, height);

		m_fullScreen = false;
		m_width = width;
		m_height = height;
	}

	// Setup close response from WM.
	XSetWMProtocols(m_display, m_window, &m_atomWmDeleteWindow, 1);
	XFlush(m_display);
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
	bool receivedEvent = false;
	XEvent evt;

	// First check for explicit Client message; cannot use masked function.
	if (XCheckTypedWindowEvent(m_display, m_window, ClientMessage, &evt))
	{
		if ((Atom)evt.xclient.data.l[0] == m_atomWmDeleteWindow)
		{
			outEvent.type = RenderEventType::Close;
			receivedEvent = true;
		}
	}

	// Then check for other events we're interested in.
	while (XCheckWindowEvent(m_display, m_window, StructureNotifyMask | FocusChangeMask, &evt))
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
				outEvent.type = RenderEventType::Resize;
				outEvent.resize.width = evt.xconfigure.width;
				outEvent.resize.height = evt.xconfigure.height;
				receivedEvent = true;
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

	return receivedEvent;
}

void Window::setWmProperty(Atom property, int32_t value)
{
	XEvent evt = { 0 };

	evt.type = ClientMessage;
	evt.xclient.window = m_window;
	evt.xclient.message_type = m_atomWmState;
	evt.xclient.format = 32;
	evt.xclient.data.l[0] = value;
	evt.xclient.data.l[1] = property;
	evt.xclient.data.l[2] = 0;
	evt.xclient.data.l[3] = 0;
	evt.xclient.data.l[4] = 0;

	XSendEvent(
		m_display,
		RootWindow(m_display, m_screen),
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&evt
	);
}

void Window::setWmProperty(Atom property1, Atom property2, int32_t value)
{
	XEvent evt = { 0 };

	evt.type = ClientMessage;
	evt.xclient.window = m_window;
	evt.xclient.message_type = m_atomWmState;
	evt.xclient.format = 32;
	evt.xclient.data.l[0] = value;
	evt.xclient.data.l[1] = property1;
	evt.xclient.data.l[2] = property2;
	evt.xclient.data.l[3] = 0;
	evt.xclient.data.l[4] = 0;

	XSendEvent(
		m_display,
		RootWindow(m_display, m_screen),
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&evt
	);
}

	}
}

