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
#include "Ui/Itf/ISystemBitmap.h"
#include "Ui/ToolForm.h"
#include "Ui/Wl/ToolFormWl.h"
#include "Ui/Wl/ToplevelWl.h"
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"

namespace traktor::ui
{
	namespace
{

void toolFormXdgPopupConfigure(void* data, xdg_popup*, int32_t, int32_t, int32_t width, int32_t height)
{
	if (width <= 0 || height <= 0)
		return;
	ToolFormWl* form = static_cast< ToolFormWl* >(data);
	WidgetData* wd = static_cast< WidgetData* >(form->getInternalHandle());
	wd->pendingWidth = width;
	wd->pendingHeight = height;
}

void toolFormXdgPopupDone(void* data, xdg_popup*)
{
	ToolFormWl* form = static_cast< ToolFormWl* >(data);
	form->endModal(DialogResult::Cancel);
	form->setVisible(false);
}

static const xdg_popup_listener s_toolFormXdgPopupListener =
{
	toolFormXdgPopupConfigure,
	toolFormXdgPopupDone
};

WidgetData* findParentXdgSurface(WidgetData* wd)
{
	for (WidgetData* w = wd; w != nullptr; w = w->parent)
	{
		if (w->xdgSurface)
			return w;
	}
	return nullptr;
}

	}

ToolFormWl::ToolFormWl(ContextWl* context, EventSubject* owner)
:	WidgetWlImpl< IToolForm >(context, owner)
,	m_result(DialogResult::Ok)
,	m_modal(false)
{
}

bool ToolFormWl::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	const int32_t c_minWidth = 16;
	const int32_t c_minHeight = 16;

	width = std::max< int32_t >(width, c_minWidth);
	height = std::max< int32_t >(height, c_minHeight);

	m_style = style;
	m_rect = Rect(0, 0, width, height);

	if (!WidgetWlImpl< IToolForm >::create(parent, style, false, true))
		return false;

	m_listenerCtx = { m_context, this };

	if ((style & ToolForm::WsDefault) != 0)
	{
		if (m_context->hasServerSideDecorations())
		{
			// SSD path.
			m_data.xdgSurface = xdg_wm_base_get_xdg_surface(m_context->getXdgWmBase(), m_data.surface);
			xdg_surface_add_listener(m_data.xdgSurface, &s_toplevelXdgSurfaceListener, &m_listenerCtx);

			m_data.xdgToplevel = xdg_surface_get_toplevel(m_data.xdgSurface);
			xdg_toplevel_add_listener(m_data.xdgToplevel, &s_toplevelXdgToplevelListener, &m_listenerCtx);

			if ((style & WsCaption) != 0)
				xdg_toplevel_set_title(m_data.xdgToplevel, wstombs(text).c_str());

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
			endModal(DialogResult::Cancel);
		});
	}
	// For popup/floating style the xdg_popup is created in setVisible(true)
	// so the position from setRect() is available at that time.

	return true;
}

void ToolFormWl::destroy()
{
	T_FATAL_ASSERT(m_modal == false);
	destroyPopup();
	WidgetWlImpl< IToolForm >::destroy();
}

void ToolFormWl::setVisible(bool visible)
{
	if (visible == m_data.visible)
		return;

	if ((m_style & ToolForm::WsDefault) != 0)
	{
		WidgetWlImpl< IToolForm >::setVisible(visible);
		return;
	}

	// Popup path: create/destroy popup on show/hide.
	if (visible)
	{
		createPopup();
		m_data.visible = true;
		m_data.mapped = true;

		SizeEvent sizeEvent(m_owner, m_rect.getSize());
		m_owner->raiseEvent(&sizeEvent);

		draw(nullptr);

		// Flush immediately so the compositor receives the buffer in the
		// same frame as the popup creation.  Without this, Mutter never
		// displays the popup because the buffer sits in the outgoing queue.
		wl_display_flush(m_context->getDisplay());

		ShowEvent showEvent(m_owner, true);
		m_owner->raiseEvent(&showEvent);
	}
	else
	{
		m_data.visible = false;
		m_data.mapped = false;
		destroyPopup();

		ShowEvent showEvent(m_owner, false);
		m_owner->raiseEvent(&showEvent);
	}
}

void ToolFormWl::setIcon(ISystemBitmap* icon)
{
}

void ToolFormWl::setLayerImage(ISystemBitmap* layerImage)
{
}

DialogResult ToolFormWl::showModal()
{
	setVisible(true);

	m_context->processPendingExposes();
	wl_display_flush(m_context->getDisplay());
	wl_display_roundtrip(m_context->getDisplay());

	m_context->pushModal(&m_data);

	for (m_modal = true; m_modal;)
		if (!Application::getInstance()->process())
			break;

	setVisible(false);
	m_context->popModal(&m_data);

	return m_result;
}

void ToolFormWl::endModal(DialogResult result)
{
	m_result = result;
	m_modal = false;
}

void ToolFormWl::createPopup()
{
	if (m_data.xdgPopup)
		return;

	WidgetData* parentXdg = m_data.parent ? findParentXdgSurface(m_data.parent) : nullptr;
	if (!parentXdg)
		return;

	// Popups have no decorations — use a raw xdg_surface (not libdecor).
	m_data.xdgSurface = xdg_wm_base_get_xdg_surface(m_context->getXdgWmBase(), m_data.surface);
	xdg_surface_add_listener(m_data.xdgSurface, &s_toplevelXdgSurfaceListener, &m_listenerCtx);

	// Convert device-pixel rect to Wayland logical coordinates for the positioner.
	const int32_t scale = m_context->getOutputScale();
	const int32_t lx = m_rect.left / scale;
	const int32_t ly = m_rect.top / scale;
	const int32_t lw = m_rect.getWidth() / scale;
	const int32_t lh = m_rect.getHeight() / scale;

	xdg_positioner* positioner = xdg_wm_base_create_positioner(m_context->getXdgWmBase());
	xdg_positioner_set_size(positioner, lw, lh);
	xdg_positioner_set_anchor_rect(positioner, lx, ly, 1, 1);
	xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_TOP_LEFT);
	xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);

	m_data.xdgPopup = xdg_surface_get_popup(m_data.xdgSurface, parentXdg->xdgSurface, positioner);
	xdg_popup_add_listener(m_data.xdgPopup, &s_toolFormXdgPopupListener, this);

	// Mutter only accepts button-press serials for xdg_popup_grab; enter or
	// release serials cause it to immediately send popup_done.
	xdg_popup_grab(m_data.xdgPopup, m_context->getSeat(), m_context->getGrabSerial());

	xdg_positioner_destroy(positioner);

	m_context->pushPopup(&m_data);

	wl_surface_commit(m_data.surface);
	wl_display_roundtrip(m_context->getDisplay());
}

void ToolFormWl::destroyPopup()
{
	if (!m_data.xdgPopup)
		return;

	// Enforce xdg-shell LIFO order: destroy all child popups above us
	// before destroying our own popup.
	m_context->destroyPopupsAbove(&m_data);

	xdg_popup_destroy(m_data.xdgPopup);
	m_data.xdgPopup = nullptr;

	if (m_data.xdgSurface)
	{
		xdg_surface_destroy(m_data.xdgSurface);
		m_data.xdgSurface = nullptr;
	}

	// Detach buffer and commit so the compositor unmaps the surface
	// immediately rather than showing a stale frame.
	wl_surface_attach(m_data.surface, nullptr, 0, 0);
	wl_surface_commit(m_data.surface);
}

}
