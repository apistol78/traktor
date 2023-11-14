/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <climits>
#include <cstring>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include "Core/Containers/AlignedVector.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Ui/Events/CloseEvent.h"
#include "Ui/Itf/ISystemBitmap.h"
#include "Ui/X11/FormX11.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

bool haveProperty(Display* display, const Atom* properties, unsigned long nproperties, const char* tokenName)
{
	Atom token = XInternAtom(display, tokenName, False);
	for (unsigned long i = 0; i < nproperties; ++i)
	{
		if (properties[i] == token)
			return true;
	}
	return false;
}

 #define MWM_DECOR_NONE          0
 #define MWM_DECOR_ALL           (1L << 0)
 #define MWM_DECOR_BORDER        (1L << 1)
 #define MWM_DECOR_RESIZEH       (1L << 2)
 #define MWM_DECOR_TITLE         (1L << 3)
 #define MWM_DECOR_MENU          (1L << 4)
 #define MWM_DECOR_MINIMIZE      (1L << 5)
 #define MWM_DECOR_MAXIMIZE      (1L << 6) 

 /* KDE decoration values */
 enum {
  KDE_noDecoration = 0,
  KDE_normalDecoration = 1,
  KDE_tinyDecoration = 2,
  KDE_noFocus = 256, 
  KDE_standaloneMenuBar = 512,
  KDE_desktopIcon = 1024 ,
  KDE_staysOnTop = 2048
 };

void wm_nodecorations(Display* dpy, Window window) {
    Atom WM_HINTS;
    int set;

    WM_HINTS = XInternAtom(dpy, "_MOTIF_WM_HINTS", True);
    if ( WM_HINTS != 0 ) {
        #define MWM_HINTS_DECORATIONS   (1L << 1)
        struct {
          unsigned long flags;
          unsigned long functions;
          unsigned long decorations;
                   long input_mode;
          unsigned long status;
        } MWMHints = { MWM_HINTS_DECORATIONS, 0,
            MWM_DECOR_NONE, 0, 0 };
        XChangeProperty(dpy, window, WM_HINTS, WM_HINTS, 32,
                        PropModeReplace, (unsigned char *)&MWMHints,
                        sizeof(MWMHints)/4);
    }
    WM_HINTS = XInternAtom(dpy, "KWM_WIN_DECORATION", True);
    if ( WM_HINTS != 0 ) {
        long KWMHints = KDE_tinyDecoration;
        XChangeProperty(dpy, window, WM_HINTS, WM_HINTS, 32,
                        PropModeReplace, (unsigned char *)&KWMHints,
                        sizeof(KWMHints)/4);
    }

    WM_HINTS = XInternAtom(dpy, "_WIN_HINTS", True);
    if ( WM_HINTS != 0 ) {
        long GNOMEHints = 0;
        XChangeProperty(dpy, window, WM_HINTS, WM_HINTS, 32,
                        PropModeReplace, (unsigned char *)&GNOMEHints,
                        sizeof(GNOMEHints)/4);
    }
    WM_HINTS = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", True);
    if ( WM_HINTS != 0 ) {
        Atom NET_WMHints[2];
        NET_WMHints[0] = XInternAtom(dpy,
            "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE", True);
        NET_WMHints[1] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NORMAL", True);
        XChangeProperty(dpy, window,
                        WM_HINTS, XA_ATOM, 32, PropModeReplace,
                        (unsigned char *)&NET_WMHints, 2);
    }
    XSetTransientForHint(dpy, window, DefaultRootWindow(dpy));
    // XUnmapWindow(dpy, window);
    // XMapWindow(dpy, window);
 }   

		}

FormX11::FormX11(Context* context, EventSubject* owner)
:	WidgetX11Impl< IForm >(context, owner)
{
}

bool FormX11::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	T_FATAL_ASSERT(parent == nullptr);

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

	// Register "delete window" window manager message.
	m_atomWmDeleteWindow = XInternAtom(m_context->getDisplay(), "WM_DELETE_WINDOW", False);

	XSetWMProtocols(m_context->getDisplay(), window, &m_atomWmDeleteWindow, 1);

	if (!WidgetX11Impl< IForm >::create(nullptr, style, window, Rect(0, 0, width, height), false, true))
		return false;

	// Remove chrome if no caption set.
	if ((style & (WsSystemBox | WsMinimizeBox | WsCloseBox | WsCaption)) == 0)
	{
	  	// Atom type = XInternAtom(m_context->getDisplay(),"_NET_WM_WINDOW_TYPE", False);
		// Atom value = XInternAtom(m_context->getDisplay(),"_NET_WM_WINDOW_TYPE_TOOLBAR", False);
		// XChangeProperty(m_context->getDisplay(), window, type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&value), 1);
		wm_nodecorations(m_context->getDisplay(), window);
	}

	// Listen to client messages when user tries to close form.
	m_context->bind(&m_data, ClientMessage, [&](XEvent& xe){
		if ((Atom)xe.xclient.data.l[0] == m_atomWmDeleteWindow)
		{
			CloseEvent closeEvent(m_owner);
			m_owner->raiseEvent(&closeEvent);
			if (!closeEvent.consumed() && !closeEvent.cancelled())
				destroy();
		}
	});

	setText(text);
	return true;
}

void FormX11::setText(const std::wstring& text)
{
	WidgetX11Impl< IForm >::setText(text);

	std::string cs = wstombs(text);
	const char* csp = cs.c_str();

	XTextProperty tp;
	XStringListToTextProperty((char**)&csp, 1, &tp);

	XSetWMName(m_context->getDisplay(), m_data.window, &tp);
}

void FormX11::setIcon(ISystemBitmap* icon)
{
	Ref< drawing::Image > ii = icon->getImage();
	ii->convert(drawing::PixelFormat::getA8R8G8B8());

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

		Ref< drawing::Image > img = ii->clone();
		img->apply(&sf);

		data.push_back(w);
		data.push_back(h);

		uint32_t o = data.size();
		data.resize(o + w * h);

		const uint32_t* src = static_cast< const uint32_t* >(img->getData());
		for (uint32_t i = 0; i < w * h; ++i)
			data[o + i] = (unsigned long)src[i];
	}

	XChangeProperty(
		m_context->getDisplay(),
		m_data.window,
		XInternAtom(m_context->getDisplay(), "_NET_WM_ICON", False),
		XA_CARDINAL, 32,
		PropModeReplace,
		(unsigned char*)data.ptr(),
		data.size()
	);

	XFlush(m_context->getDisplay());
}

void FormX11::maximize()
{
	XEvent event;
	event.xclient.type = ClientMessage;
	event.xclient.window = m_data.window;
	event.xclient.message_type = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE", False);
	event.xclient.format = 32;
	event.xclient.data.l[0] = 0;
	event.xclient.data.l[1] = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE_HIDDEN", False);
	event.xclient.data.l[2] = 0;
	XSendEvent(m_context->getDisplay(), m_context->getRootWindow(), False, SubstructureRedirectMask, &event);

	event.xclient.type = ClientMessage;
	event.xclient.window = m_data.window;
	event.xclient.message_type = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE", False);
	event.xclient.format = 32;
	event.xclient.data.l[0] = 1;
	event.xclient.data.l[1] = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE_MAXIMIZED_VERT", False);
	event.xclient.data.l[2] = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	event.xclient.data.l[3] = 0;
	XSendEvent(m_context->getDisplay(), m_context->getRootWindow(), False, SubstructureRedirectMask, &event);
}

void FormX11::minimize()
{
	XEvent event;
	event.xclient.type = ClientMessage;
	event.xclient.window = m_data.window;
	event.xclient.message_type = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE", False);
	event.xclient.format = 32;
	event.xclient.data.l[0] = 1;
	event.xclient.data.l[1] = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE_HIDDEN", False);
	event.xclient.data.l[2] = 0;
	XSendEvent(m_context->getDisplay(), m_context->getRootWindow(), False, SubstructureRedirectMask, &event);
}

void FormX11::restore()
{
	XEvent event;
	event.xclient.type = ClientMessage;
	event.xclient.window = m_data.window;
	event.xclient.message_type = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE", False);
	event.xclient.format = 32;
	event.xclient.data.l[0] = 0;
	event.xclient.data.l[1] = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE_HIDDEN", False);
	event.xclient.data.l[2] = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE_MAXIMIZED_VERT", False);
	event.xclient.data.l[3] = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	event.xclient.data.l[4] = 0;
	XSendEvent(m_context->getDisplay(), m_context->getRootWindow(), False, SubstructureRedirectMask, &event);
}

bool FormX11::isMaximized() const
{
	Atom type;
	int format;
	unsigned long nItem, bytesAfter;
	Atom *properties = nullptr;

	XGetWindowProperty(
		m_context->getDisplay(),
		m_data.window,
		XInternAtom(m_context->getDisplay(), "_NET_WM_STATE", False),
		0,
		LONG_MAX,
		False,
		XA_ATOM,
		&type,
		&format,
		&nItem,
		&bytesAfter,
		(unsigned char **)&properties
	);

	bool maximized =
		haveProperty(m_context->getDisplay(), properties, nItem, "_NET_WM_STATE_MAXIMIZED_VERT") &&
		haveProperty(m_context->getDisplay(), properties, nItem, "_NET_WM_STATE_MAXIMIZED_HORZ");

	XFree(properties);
	return maximized;
}

bool FormX11::isMinimized() const
{
	Atom type;
	int format;
	unsigned long nItem, bytesAfter;
	Atom *properties = nullptr;

	XGetWindowProperty(
		m_context->getDisplay(),
		m_data.window,
		XInternAtom(m_context->getDisplay(), "_NET_WM_STATE", False),
		0,
		LONG_MAX,
		False,
		XA_ATOM,
		&type,
		&format,
		&nItem,
		&bytesAfter,
		(unsigned char **)&properties
	);

	bool minimized = haveProperty(m_context->getDisplay(), properties, nItem, "_NET_WM_STATE_HIDDEN");

	XFree(properties);
	return minimized;
}

void FormX11::hideProgress()
{
}

void FormX11::showProgress(int32_t current, int32_t total)
{
}

	}
}

