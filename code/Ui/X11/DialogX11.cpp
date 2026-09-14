/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include "Core/Timer/Timer.h"
#include "Ui/Dialog.h"
#include "Ui/X11/Context.h"
#include "Ui/X11/DialogX11.h"
#include "Ui/X11/Timers.h"

namespace traktor::ui
{

DialogX11::DialogX11(Context* context, EventSubject* owner)
:	WidgetX11Impl< IDialog >(context, owner)
,	m_parent(0)
,	m_result(DialogResult::Ok)
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

	// Remove chrome if no caption set.
	if ((style & (WsSystemBox | WsMinimizeBox | WsCloseBox | WsCaption)) == 0)
	{
	  	Atom type = XInternAtom(m_context->getDisplay(),"_NET_WM_WINDOW_TYPE", False);
		Atom value = XInternAtom(m_context->getDisplay(),"_NET_WM_WINDOW_TYPE_DOCK", False);
		XChangeProperty(m_context->getDisplay(), window, type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&value), 1);
	}
	// Else set as dialog type.
	else
	{
	  	Atom type = XInternAtom(m_context->getDisplay(),"_NET_WM_WINDOW_TYPE", False);
		Atom value = XInternAtom(m_context->getDisplay(),"_NET_WM_WINDOW_TYPE_DIALOG", False);
		XChangeProperty(m_context->getDisplay(), window, type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&value), 1);
	}

	// Notify WM about form title.
	if ((style & WsCaption) != 0)
	{
		const std::string cs = wstombs(text);
		const char* csp = cs.c_str();

		XTextProperty tp;
		XStringListToTextProperty((char**)&csp, 1, &tp);
		XSetWMName(m_context->getDisplay(), window, &tp);
	}

	// Make dialog on top of parent.
	if (parent != nullptr)
	{
		WidgetData* parentData = static_cast< WidgetData* >(parent->getInternalHandle());
		XSetTransientForHint(m_context->getDisplay(), window, parentData->window);
	}

	// Check if we should center.
	if ((style & Dialog::WsCenterDesktop) != 0)
		m_parent = DefaultRootWindow(m_context->getDisplay());
	else if (parent != nullptr)
		m_parent = static_cast< WidgetData* >(parent->getInternalHandle())->window;

	// Register "delete window" window manager message.
	m_atomWmDeleteWindow = XInternAtom(m_context->getDisplay(), "WM_DELETE_WINDOW", False);
	XSetWMProtocols(m_context->getDisplay(), window, &m_atomWmDeleteWindow, 1);

	Rect rc(0, 0, width, height);
	if (!WidgetX11Impl< IDialog >::create(nullptr, style, window, rc, false, true))
		return false;

	m_context->bind(&m_data, ClientMessage, [=, this](XEvent& xe){
		if ((Atom)xe.xclient.data.l[0] == m_atomWmDeleteWindow)
		{
			CloseEvent closeEvent(m_owner);
			m_owner->raiseEvent(&closeEvent);
			if (!(closeEvent.consumed() && closeEvent.cancelled()))
				endModal(DialogResult::Cancel);
		}
	});

	return true;
}

void DialogX11::destroy()
{
	T_FATAL_ASSERT (m_modal == false);
	WidgetX11Impl< IDialog >::destroy();
}

void DialogX11::setIcon(ISystemBitmap* icon)
{
}

DialogResult DialogX11::showModal()
{
	setWmProperty("_NET_WM_STATE_MODAL", _NET_WM_STATE_ADD);
	setVisible(true);

	XFlush(m_context->getDisplay());

	m_context->pushModal(&m_data);

	int fd = ConnectionNumber(m_context->getDisplay());
	XEvent e;

	Timer timer;

	for (m_modal = true; m_modal; )
	{
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		struct timeval tv;
		tv.tv_usec = 1 * 1000;
		tv.tv_sec = 0;
		select(fd + 1, &fds, nullptr, nullptr, &tv);

		if (!Application::getInstance()->process())
			break;
	}

	setVisible(false);
	setWmProperty("_NET_WM_STATE_MODAL", _NET_WM_STATE_REMOVE);

	m_context->popModal(&m_data);

	return m_result;
}

void DialogX11::endModal(DialogResult result)
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
	XSetWMNormalHints(m_context->getDisplay(), m_data.window, &sh);
}

void DialogX11::setText(const std::wstring& text)
{
	const std::string cs = wstombs(text);
	const char* csp = cs.c_str();

	XTextProperty tp;
	XStringListToTextProperty((char**)&csp, 1, &tp);

	XSetWMName(m_context->getDisplay(), m_data.window, &tp);
}

void DialogX11::setVisible(bool visible)
{
	WidgetX11Impl< IDialog >::setVisible(visible);

	// Center dialog if style has been set.
	if (m_parent != 0)
	{
		Window root;
		int px, py;
		unsigned int pwidth, pheight;
		unsigned int pborder, pdepth;

		XGetGeometry(
			m_context->getDisplay(),
			m_parent,
			&root,
			&px, &py,
			&pwidth,
			&pheight,
			&pborder,
			&pdepth
		);

		Rect rc = getRect();

		XMoveWindow(
			m_context->getDisplay(),
			m_data.window,
			px + (pwidth - rc.getWidth()) / 2,
			py + (pheight - rc.getHeight()) / 2
		);
	}
}

}
