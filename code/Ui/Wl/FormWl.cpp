/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <libdecor.h>
#include "Core/Misc/TString.h"
#include "Ui/Events/CloseEvent.h"
#include "Ui/Itf/ISystemBitmap.h"
#include "Ui/Wl/FormWl.h"

namespace traktor::ui
{
	namespace
	{

void frameConfigure(struct libdecor_frame* frame, struct libdecor_configuration* configuration, void* userData)
{
	FormWl* form = static_cast< FormWl* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(form->getInternalHandle());

	int width = 0, height = 0;
	if (!libdecor_configuration_get_content_size(configuration, frame, &width, &height))
	{
		// No size provided (initial configure) — use current.
		const Rect rc = form->getRect();
		const int32_t scale = form->getContextWl()->getOutputScale();
		width = rc.getWidth() / scale;
		height = rc.getHeight() / scale;
	}

	struct libdecor_state* state = libdecor_state_new(width, height);
	libdecor_frame_commit(frame, state, configuration);
	libdecor_state_free(state);

	wd->configured = true;

	if (width > 0 && height > 0)
	{
		const int32_t scale = form->getContextWl()->getOutputScale();
		const Rect prev = form->getRect();
		const Rect next(prev.left, prev.top, prev.left + width * scale, prev.top + height * scale);

		form->setRect(next);
		form->getContextWl()->processPendingExposes();
		form->update(nullptr, true);
	}
}

void frameClose(struct libdecor_frame* frame, void* userData)
{
	FormWl* form = static_cast< FormWl* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(form->getInternalHandle());

	WlEvent e;
	e.type = WlEvtClose;
	e.surface = wd->surface;
	form->getContextWl()->dispatch(e);
}

void frameCommit(struct libdecor_frame* frame, void* userData)
{
	FormWl* form = static_cast< FormWl* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(form->getInternalHandle());
	wl_surface_commit(wd->surface);
}

void frameDismissPopup(struct libdecor_frame* frame, const char* seatName, void* userData)
{
}

static struct libdecor_frame_interface s_frameInterface = {
	frameConfigure,
	frameClose,
	frameCommit,
	frameDismissPopup,
	nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr
};

	}

FormWl::FormWl(ContextWl* context, EventSubject* owner)
:	WidgetWlImpl< IForm >(context, owner)
{
}

bool FormWl::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	T_FATAL_ASSERT(parent == nullptr);

	const int32_t c_minWidth = 16;
	const int32_t c_minHeight = 16;

	width = std::max< int32_t >(width, c_minWidth);
	height = std::max< int32_t >(height, c_minHeight);

	m_rect = Rect(0, 0, width, height);

	if (!WidgetWlImpl< IForm >::create(nullptr, style, false, true))
		return false;

	// Use libdecor to create a decorated toplevel.
	m_data.frame = libdecor_decorate(
		m_context->getLibdecor(),
		m_data.surface,
		&s_frameInterface,
		this
	);
	if (!m_data.frame)
		return false;

	// Set title.
	libdecor_frame_set_title(m_data.frame, wstombs(text).c_str());
	m_text = text;

	// Retrieve the xdg objects owned by libdecor for any code that needs them.
	m_data.xdgSurface = libdecor_frame_get_xdg_surface(m_data.frame);
	m_data.xdgToplevel = libdecor_frame_get_xdg_toplevel(m_data.frame);

	// Listen for close events via the context.
	m_context->bind(&m_data, WlEvtClose, [this](WlEvent& e) {
		CloseEvent closeEvent(m_owner);
		m_owner->raiseEvent(&closeEvent);
		if (!closeEvent.consumed() && !closeEvent.cancelled())
			destroy();
	});

	// Map the frame — this triggers the initial configure.
	libdecor_frame_map(m_data.frame);
	wl_display_roundtrip(m_context->getDisplay());

	return true;
}

void FormWl::setText(const std::wstring& text)
{
	WidgetWlImpl< IForm >::setText(text);
	if (m_data.frame)
		libdecor_frame_set_title(m_data.frame, wstombs(text).c_str());
}

void FormWl::setIcon(ISystemBitmap* icon)
{
}

void FormWl::maximize()
{
	if (m_data.frame)
	{
		libdecor_frame_set_maximized(m_data.frame);
		m_maximized = true;
		m_minimized = false;
	}
}

void FormWl::minimize()
{
	if (m_data.frame)
	{
		libdecor_frame_set_minimized(m_data.frame);
		m_minimized = true;
	}
}

void FormWl::restore()
{
	if (m_data.frame)
	{
		libdecor_frame_unset_maximized(m_data.frame);
		m_maximized = false;
		m_minimized = false;
	}
}

bool FormWl::isMaximized() const
{
	return m_maximized;
}

bool FormWl::isMinimized() const
{
	return m_minimized;
}

void FormWl::hideProgress()
{
}

void FormWl::showProgress(int32_t current, int32_t total)
{
}

}
