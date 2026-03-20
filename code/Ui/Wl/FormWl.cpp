/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/TString.h"
#include "Ui/Events/CloseEvent.h"
#include "Ui/Itf/ISystemBitmap.h"
#include "Ui/Wl/FormWl.h"
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"

namespace traktor::ui
{
	namespace
	{

void xdgSurfaceConfigure(void* data, struct xdg_surface* xdgSurface, uint32_t serial)
{
	xdg_surface_ack_configure(xdgSurface, serial);

	FormWl* form = static_cast< FormWl* >(data);
	WidgetData* wd = static_cast< WidgetData* >(form->getInternalHandle());
	wd->configured = true;

	// Apply pending size from the preceding xdg_toplevel::configure, then
	// immediately draw so the ack and the matching buffer commit happen
	// in the same event cycle.
	if (wd->pendingWidth > 0 && wd->pendingHeight > 0)
	{
		// Convert Wayland logical size to device pixels for the UI library.
		const int32_t scale = form->getContextWl()->getOutputScale();
		const Rect prev = form->getRect();
		const Rect next(prev.left, prev.top, prev.left + wd->pendingWidth * scale, prev.top + wd->pendingHeight * scale);
		wd->pendingWidth = 0;
		wd->pendingHeight = 0;

		form->setRect(next);

		// Draw all children that were queued during setRect/relayout
		// before drawing the form itself, so all subsurface buffers
		// are committed in a single batch.
		form->getContextWl()->processPendingExposes();

		form->update(nullptr, true);
	}
}

static const struct xdg_surface_listener s_xdgSurfaceListener = {
	xdgSurfaceConfigure
};

void xdgToplevelConfigure(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states)
{
	FormWl* form = static_cast< FormWl* >(data);
	WidgetData* wd = static_cast< WidgetData* >(form->getInternalHandle());

	// Store pending size; applied when xdg_surface::configure arrives.
	// width/height of 0 means the client may pick its own size.
	wd->pendingWidth = width;
	wd->pendingHeight = height;
}

void xdgToplevelClose(void* data, struct xdg_toplevel* toplevel)
{
	FormWl* form = static_cast< FormWl* >(data);
	WidgetData* wd = static_cast< WidgetData* >(form->getInternalHandle());

	WlEvent e;
	e.type = WlEvtClose;
	e.surface = wd->surface;
	form->getContextWl()->dispatch(e);
}

static const struct xdg_toplevel_listener s_xdgToplevelListener = {
	xdgToplevelConfigure,
	xdgToplevelClose
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

	// Create xdg_surface and xdg_toplevel.
	m_data.xdgSurface = xdg_wm_base_get_xdg_surface(m_context->getXdgWmBase(), m_data.surface);
	xdg_surface_add_listener(m_data.xdgSurface, &s_xdgSurfaceListener, this);

	m_data.xdgToplevel = xdg_surface_get_toplevel(m_data.xdgSurface);
	xdg_toplevel_add_listener(m_data.xdgToplevel, &s_xdgToplevelListener, this);

	// Set title.
	xdg_toplevel_set_title(m_data.xdgToplevel, wstombs(text).c_str());
	m_text = text;

	// Request server-side decorations (title bar, close button, etc.).
	struct zxdg_decoration_manager_v1* decorationManager = m_context->getDecorationManager();
	if (decorationManager)
	{
		m_data.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(decorationManager, m_data.xdgToplevel);
		zxdg_toplevel_decoration_v1_set_mode(m_data.decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
	}

	// Listen for close events via the context.
	m_context->bind(&m_data, WlEvtClose, [this](WlEvent& e) {
		CloseEvent closeEvent(m_owner);
		m_owner->raiseEvent(&closeEvent);
		if (!closeEvent.consumed() && !closeEvent.cancelled())
			destroy();
	});

	// Commit the surface to trigger the initial configure.
	wl_surface_commit(m_data.surface);
	wl_display_roundtrip(m_context->getDisplay());

	return true;
}

void FormWl::setText(const std::wstring& text)
{
	WidgetWlImpl< IForm >::setText(text);
	if (m_data.xdgToplevel)
		xdg_toplevel_set_title(m_data.xdgToplevel, wstombs(text).c_str());
}

void FormWl::setIcon(ISystemBitmap* icon)
{
	// Wayland doesn't have a standard way to set window icons.
	// Some compositors support it through desktop entry files.
}

void FormWl::maximize()
{
	if (m_data.xdgToplevel)
	{
		xdg_toplevel_set_maximized(m_data.xdgToplevel);
		m_maximized = true;
		m_minimized = false;
	}
}

void FormWl::minimize()
{
	if (m_data.xdgToplevel)
	{
		xdg_toplevel_set_minimized(m_data.xdgToplevel);
		m_minimized = true;
	}
}

void FormWl::restore()
{
	if (m_data.xdgToplevel)
	{
		xdg_toplevel_unset_maximized(m_data.xdgToplevel);
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
