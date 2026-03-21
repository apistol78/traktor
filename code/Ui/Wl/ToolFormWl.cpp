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
#include "Ui/Itf/ISystemBitmap.h"
#include "Ui/ToolForm.h"
#include "Ui/Wl/ToolFormWl.h"
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"

namespace traktor::ui
{
	namespace
	{

// --- libdecor frame callbacks (WsDefault / dialog-like) ---

void toolFormFrameConfigure(struct libdecor_frame* frame, struct libdecor_configuration* configuration, void* userData)
{
	ToolFormWl* form = static_cast< ToolFormWl* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(form->getInternalHandle());

	int width = 0, height = 0;
	if (!libdecor_configuration_get_content_size(configuration, frame, &width, &height))
	{
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

void toolFormFrameClose(struct libdecor_frame* frame, void* userData)
{
	ToolFormWl* form = static_cast< ToolFormWl* >(userData);
	form->endModal(DialogResult::Cancel);
}

void toolFormFrameCommit(struct libdecor_frame* frame, void* userData)
{
	ToolFormWl* form = static_cast< ToolFormWl* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(form->getInternalHandle());
	wl_surface_commit(wd->surface);
}

void toolFormFrameDismissPopup(struct libdecor_frame* frame, const char* seatName, void* userData)
{
}

static struct libdecor_frame_interface s_toolFormFrameInterface = {
	toolFormFrameConfigure,
	toolFormFrameClose,
	toolFormFrameCommit,
	toolFormFrameDismissPopup,
	nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr
};

// ============================================================
// SSD path — xdg_toplevel listeners for WsDefault
// ============================================================

void toolFormSsdToplevelConfigure(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states)
{
	ToolFormWl* form = static_cast< ToolFormWl* >(data);
	WidgetData* wd = static_cast< WidgetData* >(form->getInternalHandle());
	wd->pendingWidth = width;
	wd->pendingHeight = height;
}

void toolFormSsdToplevelClose(void* data, struct xdg_toplevel* toplevel)
{
	ToolFormWl* form = static_cast< ToolFormWl* >(data);
	form->endModal(DialogResult::Cancel);
}

static const struct xdg_toplevel_listener s_toolFormXdgToplevelListener = {
	toolFormSsdToplevelConfigure,
	toolFormSsdToplevelClose
};

// --- Popup xdg_surface listener (non-WsDefault / floating) ---

void toolFormPopupXdgSurfaceConfigure(void* data, struct xdg_surface* xdgSurface, uint32_t serial)
{
	xdg_surface_ack_configure(xdgSurface, serial);

	ToolFormWl* form = static_cast< ToolFormWl* >(data);
	WidgetData* wd = static_cast< WidgetData* >(form->getInternalHandle());
	wd->configured = true;

	if (wd->pendingWidth > 0 && wd->pendingHeight > 0)
	{
		const int32_t scale = form->getContextWl()->getOutputScale();
		const Rect prev = form->getRect();
		const Rect next(prev.left, prev.top, prev.left + wd->pendingWidth * scale, prev.top + wd->pendingHeight * scale);
		wd->pendingWidth = 0;
		wd->pendingHeight = 0;

		form->setRect(next);
		form->getContextWl()->processPendingExposes();
		form->update(nullptr, true);
	}
}

static const struct xdg_surface_listener s_toolFormPopupXdgSurfaceListener = {
	toolFormPopupXdgSurfaceConfigure
};

void toolFormXdgPopupConfigure(void* data, struct xdg_popup* popup, int32_t x, int32_t y, int32_t width, int32_t height)
{
	if (width > 0 && height > 0)
	{
		ToolFormWl* form = static_cast< ToolFormWl* >(data);
		WidgetData* wd = static_cast< WidgetData* >(form->getInternalHandle());
		wd->pendingWidth = width;
		wd->pendingHeight = height;
	}
}

void toolFormXdgPopupDone(void* data, struct xdg_popup* popup)
{
	ToolFormWl* form = static_cast< ToolFormWl* >(data);
	form->endModal(DialogResult::Cancel);
}

static const struct xdg_popup_listener s_toolFormXdgPopupListener = {
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

	if ((style & ToolForm::WsDefault) != 0)
	{
		if (m_context->hasServerSideDecorations())
		{
			// SSD path.
			m_data.xdgSurface = xdg_wm_base_get_xdg_surface(m_context->getXdgWmBase(), m_data.surface);
			xdg_surface_add_listener(m_data.xdgSurface, &s_toolFormPopupXdgSurfaceListener, this);

			m_data.xdgToplevel = xdg_surface_get_toplevel(m_data.xdgSurface);
			xdg_toplevel_add_listener(m_data.xdgToplevel, &s_toolFormXdgToplevelListener, this);

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
			wl_display_roundtrip(m_context->getDisplay());
		}
		else
		{
			// libdecor fallback.
			m_data.frame = libdecor_decorate(
				m_context->getLibdecor(),
				m_data.surface,
				&s_toolFormFrameInterface,
				this
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
			wl_display_roundtrip(m_context->getDisplay());
		}
	}
	// For popup/floating style, the xdg_popup is created in setVisible(true)
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

	if ((m_style & ToolForm::WsDefault) == 0)
	{
		// Popup path: create/destroy popup on show/hide.
		if (visible)
		{
			createPopup();
			m_data.visible = true;
			m_data.mapped = true;

			SizeEvent sizeEvent(m_owner, m_rect.getSize());
			m_owner->raiseEvent(&sizeEvent);

			if (m_data.mapped)
				draw(nullptr);

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
	else
	{
		WidgetWlImpl< IToolForm >::setVisible(visible);
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

	// Create xdg_surface for the popup (not using libdecor — popups have no decorations).
	m_data.xdgSurface = xdg_wm_base_get_xdg_surface(m_context->getXdgWmBase(), m_data.surface);
	xdg_surface_add_listener(m_data.xdgSurface, &s_toolFormPopupXdgSurfaceListener, this);

	// Convert device-pixel rect to Wayland logical coordinates for the positioner.
	const int32_t scale = m_context->getOutputScale();
	struct xdg_positioner* positioner = xdg_wm_base_create_positioner(m_context->getXdgWmBase());
	xdg_positioner_set_size(positioner, m_rect.getWidth() / scale, m_rect.getHeight() / scale);
	xdg_positioner_set_anchor_rect(positioner, m_rect.left / scale, m_rect.top / scale, 1, 1);
	xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_TOP_LEFT);
	xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);

	m_data.xdgPopup = xdg_surface_get_popup(m_data.xdgSurface, parentXdg->xdgSurface, positioner);
	xdg_popup_add_listener(m_data.xdgPopup, &s_toolFormXdgPopupListener, this);

	xdg_popup_grab(m_data.xdgPopup, m_context->getSeat(), m_context->getPointerSerial());

	xdg_positioner_destroy(positioner);

	wl_surface_commit(m_data.surface);
	wl_display_roundtrip(m_context->getDisplay());
}

void ToolFormWl::destroyPopup()
{
	if (m_data.xdgPopup)
	{
		xdg_popup_destroy(m_data.xdgPopup);
		m_data.xdgPopup = nullptr;
	}

	// Only destroy xdg_surface if it's not owned by libdecor.
	if (m_data.xdgSurface && !m_data.frame)
	{
		xdg_surface_destroy(m_data.xdgSurface);
		m_data.xdgSurface = nullptr;
	}
}

}
