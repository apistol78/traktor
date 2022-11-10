/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <X11/Xatom.h>
#include "Core/Timer/Timer.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Ui/Itf/ISystemBitmap.h"
#include "Ui/X11/ToolFormX11.h"

namespace traktor
{
	namespace ui
	{

ToolFormX11::ToolFormX11(Context* context, EventSubject* owner)
:	WidgetX11Impl< IToolForm >(context, owner)
,	m_result(DialogResult::Ok)
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

void ToolFormX11::setIcon(ISystemBitmap* icon)
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

DialogResult ToolFormX11::showModal()
{
	setWmProperty("_NET_WM_STATE_MODAL", _NET_WM_STATE_ADD);
	setVisible(true);

	XFlush(m_context->getDisplay());

	m_context->pushModal(&m_data);

	for (m_modal = true; m_modal; )
	{
		if (!Application::getInstance()->process())
			break;
	}

	setVisible(false);
	setWmProperty("_NET_WM_STATE_MODAL", _NET_WM_STATE_REMOVE);

	m_context->popModal(&m_data);

	return m_result;
}

void ToolFormX11::endModal(DialogResult result)
{
	m_result = result;
	m_modal = false;
}

	}
}

