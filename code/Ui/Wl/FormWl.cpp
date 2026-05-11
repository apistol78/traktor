/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
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
#include "Ui/Wl/ToplevelWl.h"
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"

namespace traktor::ui
{

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

	m_listenerCtx = { m_context, this };

	if (m_context->hasServerSideDecorations())
	{
		// SSD path — compositor provides title bar / close button.
		m_data.xdgSurface = xdg_wm_base_get_xdg_surface(m_context->getXdgWmBase(), m_data.surface);
		xdg_surface_add_listener(m_data.xdgSurface, &s_toplevelXdgSurfaceListener, &m_listenerCtx);

		m_data.xdgToplevel = xdg_surface_get_toplevel(m_data.xdgSurface);
		xdg_toplevel_add_listener(m_data.xdgToplevel, &s_toplevelXdgToplevelListener, &m_listenerCtx);

		xdg_toplevel_set_title(m_data.xdgToplevel, wstombs(text).c_str());

		m_data.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
			m_context->getDecorationManager(), m_data.xdgToplevel);
		zxdg_toplevel_decoration_v1_set_mode(
			m_data.decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);

		wl_surface_commit(m_data.surface);
	}
	else
	{
		// libdecor fallback — client-side decorations.
		m_data.frame = libdecor_decorate(
			m_context->getLibdecor(),
			m_data.surface,
			&s_toplevelLibdecorFrameInterface,
			&m_listenerCtx
		);
		if (!m_data.frame)
			return false;

		libdecor_frame_set_title(m_data.frame, wstombs(text).c_str());

		m_data.xdgSurface = libdecor_frame_get_xdg_surface(m_data.frame);
		m_data.xdgToplevel = libdecor_frame_get_xdg_toplevel(m_data.frame);

		libdecor_frame_map(m_data.frame);
	}

	wl_display_roundtrip(m_context->getDisplay());

	m_text = text;

	m_context->bind(&m_data, WlEvtClose, [this](WlEvent& e) {
		CloseEvent closeEvent(m_owner);
		m_owner->raiseEvent(&closeEvent);
		if (!closeEvent.consumed() && !closeEvent.cancelled())
			destroy();
	});

	return true;
}

void FormWl::setText(const std::wstring& text)
{
	WidgetWlImpl< IForm >::setText(text);
	if (m_data.frame)
		libdecor_frame_set_title(m_data.frame, wstombs(text).c_str());
	else if (m_data.xdgToplevel)
		xdg_toplevel_set_title(m_data.xdgToplevel, wstombs(text).c_str());
}

void FormWl::setIcon(ISystemBitmap* icon)
{
}

void FormWl::maximize()
{
	if (m_data.frame)
		libdecor_frame_set_maximized(m_data.frame);
	else if (m_data.xdgToplevel)
		xdg_toplevel_set_maximized(m_data.xdgToplevel);
	m_maximized = true;
	m_minimized = false;
}

void FormWl::minimize()
{
	if (m_data.frame)
		libdecor_frame_set_minimized(m_data.frame);
	else if (m_data.xdgToplevel)
		xdg_toplevel_set_minimized(m_data.xdgToplevel);
	m_minimized = true;
}

void FormWl::restore()
{
	if (m_data.frame)
		libdecor_frame_unset_maximized(m_data.frame);
	else if (m_data.xdgToplevel)
		xdg_toplevel_unset_maximized(m_data.xdgToplevel);
	m_maximized = false;
	m_minimized = false;
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
