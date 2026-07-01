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
#include "Ui/Dialog.h"
#include "Ui/Wl/ContextWl.h"
#include "Ui/Wl/DialogWl.h"
#include "Ui/Wl/ToplevelWl.h"
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"

namespace traktor::ui
{
	namespace
{

void dialogPopupDone(void* data, xdg_popup* popup)
{
	DialogWl* dialog = static_cast< DialogWl* >(data);
	dialog->endModal(DialogResult::Cancel);
}

void dialogPopupConfigure(void*, xdg_popup*, int32_t, int32_t, int32_t, int32_t)
{
}

static const xdg_popup_listener s_dialogPopupListener =
{
	dialogPopupConfigure,
	dialogPopupDone
};

WidgetData* findAnchorXdgSurface(WidgetData* wd)
{
	for (WidgetData* w = wd; w != nullptr; w = w->parent)
	{
		if (w->xdgSurface)
			return w;
	}
	return nullptr;
}

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

	m_listenerCtx = { m_context, this };

	const bool wantDecorations = (style & (WsResizable | WsCaption | WsSystemBox | WsMinimizeBox | WsMaximizeBox)) != 0;

	if (!wantDecorations)
	{
		// Decoration-less dialog (e.g. splash screen).
		m_data.xdgSurface = xdg_wm_base_get_xdg_surface(m_context->getXdgWmBase(), m_data.surface);
		xdg_surface_add_listener(m_data.xdgSurface, &s_toplevelXdgSurfaceListener, &m_listenerCtx);

		WidgetData* parentData = (parent != nullptr) ? static_cast< WidgetData* >(parent->getInternalHandle()) : nullptr;
		WidgetData* anchorData = parentData ? findAnchorXdgSurface(parentData) : nullptr;

		if ((style & Dialog::WsCenterDesktop) != 0 && anchorData)
		{
			// Wayland disallows absolute toplevel positioning; use an xdg_popup
			// anchored to the parent to center the dialog on the output.
			const int32_t scale = m_context->getOutputScale();
			const int32_t pw = m_context->getOutputWidth() / scale;
			const int32_t ph = m_context->getOutputHeight() / scale;
			const int32_t dw = m_rect.getWidth() / scale;
			const int32_t dh = m_rect.getHeight() / scale;
			const int32_t cx = (pw - dw) / 2;
			const int32_t cy = (ph - dh) / 2;

			xdg_positioner* positioner = xdg_wm_base_create_positioner(m_context->getXdgWmBase());
			xdg_positioner_set_size(positioner, dw, dh);
			xdg_positioner_set_anchor_rect(positioner, cx, cy, 1, 1);
			xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_TOP_LEFT);
			xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);

			m_data.xdgPopup = xdg_surface_get_popup(m_data.xdgSurface, anchorData->xdgSurface, positioner);
			xdg_popup_add_listener(m_data.xdgPopup, &s_dialogPopupListener, this);
			xdg_positioner_destroy(positioner);
		}
		else
		{
			m_data.xdgToplevel = xdg_surface_get_toplevel(m_data.xdgSurface);
			xdg_toplevel_add_listener(m_data.xdgToplevel, &s_toplevelXdgToplevelListener, &m_listenerCtx);

			xdg_toplevel_set_app_id(m_data.xdgToplevel, m_context->getAppId());

			if (anchorData && anchorData->xdgToplevel)
				xdg_toplevel_set_parent(m_data.xdgToplevel, anchorData->xdgToplevel);
		}

		wl_surface_commit(m_data.surface);
	}
	else if (m_context->hasServerSideDecorations())
	{
		// SSD path.
		m_data.xdgSurface = xdg_wm_base_get_xdg_surface(m_context->getXdgWmBase(), m_data.surface);
		xdg_surface_add_listener(m_data.xdgSurface, &s_toplevelXdgSurfaceListener, &m_listenerCtx);

		m_data.xdgToplevel = xdg_surface_get_toplevel(m_data.xdgSurface);
		xdg_toplevel_add_listener(m_data.xdgToplevel, &s_toplevelXdgToplevelListener, &m_listenerCtx);

		if ((style & WsCaption) != 0)
			xdg_toplevel_set_title(m_data.xdgToplevel, wstombs(text).c_str());
		xdg_toplevel_set_app_id(m_data.xdgToplevel, m_context->getAppId());

		m_data.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
			m_context->getDecorationManager(), m_data.xdgToplevel);
		zxdg_toplevel_decoration_v1_set_mode(
			m_data.decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);

		if (parent != nullptr)
		{
			WidgetData* parentData = static_cast< WidgetData* >(parent->getInternalHandle());
			if (parentData->xdgToplevel)
				xdg_toplevel_set_parent(m_data.xdgToplevel, parentData->xdgToplevel);
		}

		wl_surface_commit(m_data.surface);
	}
	else
	{
		// libdecor fallback.
		m_data.frame = libdecor_decorate(
			m_context->getLibdecor(),
			m_data.surface,
			&s_toplevelLibdecorFrameInterface,
			&m_listenerCtx
		);
		if (!m_data.frame)
			return false;

		if ((style & WsCaption) != 0)
			libdecor_frame_set_title(m_data.frame, wstombs(text).c_str());
		libdecor_frame_set_app_id(m_data.frame, m_context->getAppId());

		m_data.xdgSurface = libdecor_frame_get_xdg_surface(m_data.frame);
		m_data.xdgToplevel = libdecor_frame_get_xdg_toplevel(m_data.frame);

		if (parent != nullptr)
		{
			WidgetData* parentData = static_cast< WidgetData* >(parent->getInternalHandle());
			if (parentData->frame)
				libdecor_frame_set_parent(m_data.frame, parentData->frame);
		}

		libdecor_frame_map(m_data.frame);
	}

	wl_display_roundtrip(m_context->getDisplay());

	m_context->bind(&m_data, WlEvtClose, [this](WlEvent& e) {
		// Cache modal state before raising the event — a non-modal handler
		// may destroy `this` from inside raiseEvent, after which we must not
		// touch the object.  Modal dialogs own their showModal() stack frame
		// and cannot legally be destroyed from inside the close handler.
		const bool wasModal = m_modal;
		CloseEvent closeEvent(m_owner);
		m_owner->raiseEvent(&closeEvent);
		if (wasModal && !(closeEvent.consumed() && closeEvent.cancelled()))
			endModal(DialogResult::Cancel);
	});

	return true;
}

void DialogWl::destroy()
{
	T_FATAL_ASSERT(m_modal == false);
	WidgetWlImpl< IDialog >::destroy();
}

void DialogWl::setIcon(ISystemBitmap* icon)
{
	// Wayland xdg-shell has no per-window icon protocol; the compositor
	// resolves the icon via app_id → .desktop file (set in ContextWl).
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
	const int32_t scale = m_context->getOutputScale();
	const int32_t lw = minSize.cx / scale;
	const int32_t lh = minSize.cy / scale;

	if (m_data.frame)
		libdecor_frame_set_min_content_size(m_data.frame, lw, lh);
	else if (m_data.xdgToplevel)
		xdg_toplevel_set_min_size(m_data.xdgToplevel, lw, lh);
}

void DialogWl::setText(const std::wstring& text)
{
	if (m_data.frame)
		libdecor_frame_set_title(m_data.frame, wstombs(text).c_str());
	else if (m_data.xdgToplevel)
		xdg_toplevel_set_title(m_data.xdgToplevel, wstombs(text).c_str());
}

void DialogWl::setVisible(bool visible)
{
	WidgetWlImpl< IDialog >::setVisible(visible);
}

}
