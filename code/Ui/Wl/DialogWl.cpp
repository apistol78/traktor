/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <libdecor.h>
#include "Core/Timer/Timer.h"
#include "Core/Misc/TString.h"
#include "Ui/Dialog.h"
#include "Ui/Wl/ContextWl.h"
#include "Ui/Wl/DialogWl.h"
#include "Ui/Wl/Timers.h"

namespace traktor::ui
{
	namespace
	{

void dialogFrameConfigure(struct libdecor_frame* frame, struct libdecor_configuration* configuration, void* userData)
{
	DialogWl* dialog = static_cast< DialogWl* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(dialog->getInternalHandle());

	int width = 0, height = 0;
	if (!libdecor_configuration_get_content_size(configuration, frame, &width, &height))
	{
		const Rect rc = dialog->getRect();
		const int32_t scale = dialog->getContextWl()->getOutputScale();
		width = rc.getWidth() / scale;
		height = rc.getHeight() / scale;
	}

	struct libdecor_state* state = libdecor_state_new(width, height);
	libdecor_frame_commit(frame, state, configuration);
	libdecor_state_free(state);

	wd->configured = true;

	if (width > 0 && height > 0)
	{
		const int32_t scale = dialog->getContextWl()->getOutputScale();
		const Rect prev = dialog->getRect();
		const Rect next(prev.left, prev.top, prev.left + width * scale, prev.top + height * scale);

		dialog->setRect(next);
		dialog->getContextWl()->processPendingExposes();
		dialog->update(nullptr, true);
	}
}

void dialogFrameClose(struct libdecor_frame* frame, void* userData)
{
	DialogWl* dialog = static_cast< DialogWl* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(dialog->getInternalHandle());

	WlEvent e;
	e.type = WlEvtClose;
	e.surface = wd->surface;
	dialog->getContextWl()->dispatch(e);
}

void dialogFrameCommit(struct libdecor_frame* frame, void* userData)
{
	DialogWl* dialog = static_cast< DialogWl* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(dialog->getInternalHandle());
	wl_surface_commit(wd->surface);
}

void dialogFrameDismissPopup(struct libdecor_frame* frame, const char* seatName, void* userData)
{
}

static struct libdecor_frame_interface s_dialogFrameInterface = {
	dialogFrameConfigure,
	dialogFrameClose,
	dialogFrameCommit,
	dialogFrameDismissPopup,
	nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr
};

	}

DialogWl::DialogWl(ContextWl* context, EventSubject* owner)
:	WidgetWlImpl< IDialog >(context, owner)
,	m_result(DialogResult::Ok)
,	m_modal(false)
{
}

bool DialogWl::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	const int32_t c_minWidth = 16;
	const int32_t c_minHeight = 16;

	width = std::max< int32_t >(width, c_minWidth);
	height = std::max< int32_t >(height, c_minHeight);

	m_rect = Rect(0, 0, width, height);

	if (!WidgetWlImpl< IDialog >::create(nullptr, style, false, true))
		return false;

	// Use libdecor to create a decorated toplevel.
	m_data.frame = libdecor_decorate(
		m_context->getLibdecor(),
		m_data.surface,
		&s_dialogFrameInterface,
		this
	);
	if (!m_data.frame)
		return false;

	if ((style & WsCaption) != 0)
		libdecor_frame_set_title(m_data.frame, wstombs(text).c_str());

	// Retrieve xdg objects owned by libdecor.
	m_data.xdgSurface = libdecor_frame_get_xdg_surface(m_data.frame);
	m_data.xdgToplevel = libdecor_frame_get_xdg_toplevel(m_data.frame);

	// Make dialog appear on top of parent.
	if (parent != nullptr)
	{
		WidgetData* parentData = static_cast< WidgetData* >(parent->getInternalHandle());
		if (parentData->frame)
			libdecor_frame_set_parent(m_data.frame, parentData->frame);
	}

	m_context->bind(&m_data, WlEvtClose, [this](WlEvent& e) {
		CloseEvent closeEvent(m_owner);
		m_owner->raiseEvent(&closeEvent);
		if (!(closeEvent.consumed() && closeEvent.cancelled()))
			endModal(DialogResult::Cancel);
	});

	libdecor_frame_map(m_data.frame);
	wl_display_roundtrip(m_context->getDisplay());

	return true;
}

void DialogWl::destroy()
{
	T_FATAL_ASSERT(m_modal == false);
	WidgetWlImpl< IDialog >::destroy();
}

void DialogWl::setIcon(ISystemBitmap* icon)
{
}

DialogResult DialogWl::showModal()
{
	setVisible(true);

	m_context->processPendingExposes();
	wl_display_flush(m_context->getDisplay());
	wl_display_roundtrip(m_context->getDisplay());

	m_context->pushModal(&m_data);

	for (m_modal = true; m_modal; )
	{
		if (!Application::getInstance()->process())
			break;
	}

	setVisible(false);
	m_context->popModal(&m_data);

	return m_result;
}

void DialogWl::endModal(DialogResult result)
{
	m_result = result;
	m_modal = false;
}

void DialogWl::setMinSize(const Size& minSize)
{
	if (m_data.frame)
	{
		const int32_t scale = m_context->getOutputScale();
		libdecor_frame_set_min_content_size(m_data.frame, minSize.cx / scale, minSize.cy / scale);
	}
}

void DialogWl::setText(const std::wstring& text)
{
	if (m_data.frame)
		libdecor_frame_set_title(m_data.frame, wstombs(text).c_str());
}

void DialogWl::setVisible(bool visible)
{
	WidgetWlImpl< IDialog >::setVisible(visible);
}

}
