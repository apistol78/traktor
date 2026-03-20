/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

// xdg_surface listener (shared by both toplevel and popup paths).
void toolFormXdgSurfaceConfigure(void* data, struct xdg_surface* xdgSurface, uint32_t serial)
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

static const struct xdg_surface_listener s_toolFormXdgSurfaceListener = {
	toolFormXdgSurfaceConfigure
};

// --- Toplevel listeners (WsDefault / dialog-like) ---

void toolFormXdgToplevelConfigure(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states)
{
	ToolFormWl* form = static_cast< ToolFormWl* >(data);
	WidgetData* wd = static_cast< WidgetData* >(form->getInternalHandle());
	wd->pendingWidth = width;
	wd->pendingHeight = height;
}

void toolFormXdgToplevelClose(void* data, struct xdg_toplevel* toplevel)
{
	ToolFormWl* form = static_cast< ToolFormWl* >(data);
	form->endModal(DialogResult::Cancel);
}

static const struct xdg_toplevel_listener s_toolFormXdgToplevelListener = {
	toolFormXdgToplevelConfigure,
	toolFormXdgToplevelClose
};

// --- Popup listeners (floating containers) ---

void toolFormXdgPopupConfigure(void* data, struct xdg_popup* popup, int32_t x, int32_t y, int32_t width, int32_t height)
{
	// The compositor may adjust the popup position/size.
	// If it gave us non-zero dimensions, apply them.
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

// Walk parent chain to find the nearest ancestor with an xdg_surface
// (popups must be parented to an xdg_surface, not a bare wl_surface).
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

	// Create xdg_surface (common for both toplevel and popup paths).
	m_data.xdgSurface = xdg_wm_base_get_xdg_surface(m_context->getXdgWmBase(), m_data.surface);
	xdg_surface_add_listener(m_data.xdgSurface, &s_toolFormXdgSurfaceListener, this);

	if ((style & ToolForm::WsDefault) != 0)
	{
		// Dialog-like tool form: use xdg_toplevel with decorations.
		m_data.xdgToplevel = xdg_surface_get_toplevel(m_data.xdgSurface);
		xdg_toplevel_add_listener(m_data.xdgToplevel, &s_toolFormXdgToplevelListener, this);

		if ((style & WsCaption) != 0)
			xdg_toplevel_set_title(m_data.xdgToplevel, wstombs(text).c_str());

		// Request server-side decorations.
		struct zxdg_decoration_manager_v1* decorationManager = m_context->getDecorationManager();
		if (decorationManager)
		{
			m_data.decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(decorationManager, m_data.xdgToplevel);
			zxdg_toplevel_decoration_v1_set_mode(m_data.decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
		}

		// Make tool form on top of parent.
		if (parent != nullptr)
		{
			WidgetData* parentData = static_cast< WidgetData* >(parent->getInternalHandle());
			if (parentData->xdgToplevel)
				xdg_toplevel_set_parent(m_data.xdgToplevel, parentData->xdgToplevel);
		}

		wl_surface_commit(m_data.surface);
		wl_display_roundtrip(m_context->getDisplay());
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
		// Toplevel path: use base implementation.
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

	// Ensure all child subsurfaces have buffers committed before
	// activating the modal filter.
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

	// Find the nearest parent with an xdg_surface to anchor the popup.
	WidgetData* parentXdg = m_data.parent ? findParentXdgSurface(m_data.parent) : nullptr;
	if (!parentXdg)
		return;

	// Create positioner: place popup at (m_rect.left, m_rect.top) relative
	// to the parent surface.
	// Convert device-pixel rect to Wayland logical coordinates for the positioner.
	const int32_t scale = m_context->getOutputScale();
	struct xdg_positioner* positioner = xdg_wm_base_create_positioner(m_context->getXdgWmBase());
	xdg_positioner_set_size(positioner, m_rect.getWidth() / scale, m_rect.getHeight() / scale);
	xdg_positioner_set_anchor_rect(positioner, m_rect.left / scale, m_rect.top / scale, 1, 1);
	xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_TOP_LEFT);
	xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);

	m_data.xdgPopup = xdg_surface_get_popup(m_data.xdgSurface, parentXdg->xdgSurface, positioner);
	xdg_popup_add_listener(m_data.xdgPopup, &s_toolFormXdgPopupListener, this);

	// Grab input so the popup receives pointer/keyboard events.
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
}

}
