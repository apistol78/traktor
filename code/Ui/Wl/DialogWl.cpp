/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Timer/Timer.h"
#include "Core/Misc/TString.h"
#include "Ui/Dialog.h"
#include "Ui/Wl/ContextWl.h"
#include "Ui/Wl/DialogWl.h"
#include "Ui/Wl/Timers.h"
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"

namespace traktor::ui
{
	namespace
	{

void dialogXdgSurfaceConfigure(void* data, struct xdg_surface* xdgSurface, uint32_t serial)
{
	xdg_surface_ack_configure(xdgSurface, serial);

	DialogWl* dialog = static_cast< DialogWl* >(data);
	WidgetData* wd = static_cast< WidgetData* >(dialog->getInternalHandle());
	wd->configured = true;

	if (wd->pendingWidth > 0 && wd->pendingHeight > 0)
	{
		const int32_t scale = dialog->getContextWl()->getOutputScale();
		const Rect prev = dialog->getRect();
		const Rect next(prev.left, prev.top, prev.left + wd->pendingWidth * scale, prev.top + wd->pendingHeight * scale);
		wd->pendingWidth = 0;
		wd->pendingHeight = 0;

		dialog->setRect(next);
		dialog->getContextWl()->processPendingExposes();
		dialog->update(nullptr, true);
	}
}

static const struct xdg_surface_listener s_dialogXdgSurfaceListener = {
	dialogXdgSurfaceConfigure
};

void dialogXdgToplevelConfigure(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states)
{
	DialogWl* dialog = static_cast< DialogWl* >(data);
	WidgetData* wd = static_cast< WidgetData* >(dialog->getInternalHandle());
	wd->pendingWidth = width;
	wd->pendingHeight = height;
}

void dialogXdgToplevelClose(void* data, struct xdg_toplevel* toplevel)
{
	DialogWl* dialog = static_cast< DialogWl* >(data);
	WidgetData* wd = static_cast< WidgetData* >(dialog->getInternalHandle());

	WlEvent e;
	e.type = WlEvtClose;
	e.surface = wd->surface;
	dialog->getContextWl()->dispatch(e);
}

static const struct xdg_toplevel_listener s_dialogXdgToplevelListener = {
	dialogXdgToplevelConfigure,
	dialogXdgToplevelClose
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

	m_data.xdgSurface = xdg_wm_base_get_xdg_surface(m_context->getXdgWmBase(), m_data.surface);
	xdg_surface_add_listener(m_data.xdgSurface, &s_dialogXdgSurfaceListener, this);

	m_data.xdgToplevel = xdg_surface_get_toplevel(m_data.xdgSurface);
	xdg_toplevel_add_listener(m_data.xdgToplevel, &s_dialogXdgToplevelListener, this);

	if ((style & WsCaption) != 0)
		xdg_toplevel_set_title(m_data.xdgToplevel, wstombs(text).c_str());

	// Request server-side decorations.
	struct zxdg_decoration_manager_v1* decorationManager = m_context->getDecorationManager();
	if (decorationManager)
	{
		m_data.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(decorationManager, m_data.xdgToplevel);
		zxdg_toplevel_decoration_v1_set_mode(m_data.decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
	}

	// Make dialog appear on top of parent.
	if (parent != nullptr)
	{
		WidgetData* parentData = static_cast< WidgetData* >(parent->getInternalHandle());
		if (parentData->xdgToplevel)
			xdg_toplevel_set_parent(m_data.xdgToplevel, parentData->xdgToplevel);
	}

	m_context->bind(&m_data, WlEvtClose, [this](WlEvent& e) {
		CloseEvent closeEvent(m_owner);
		m_owner->raiseEvent(&closeEvent);
		if (!(closeEvent.consumed() && closeEvent.cancelled()))
			endModal(DialogResult::Cancel);
	});

	wl_surface_commit(m_data.surface);
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

	// Ensure all child subsurfaces have buffers committed before
	// activating the modal filter.
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
	if (m_data.xdgToplevel)
		xdg_toplevel_set_min_size(m_data.xdgToplevel, minSize.cx, minSize.cy);
}

void DialogWl::setText(const std::wstring& text)
{
	if (m_data.xdgToplevel)
		xdg_toplevel_set_title(m_data.xdgToplevel, wstombs(text).c_str());
}

void DialogWl::setVisible(bool visible)
{
	WidgetWlImpl< IDialog >::setVisible(visible);
}

}
