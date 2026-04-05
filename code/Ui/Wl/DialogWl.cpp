/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
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
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"

namespace traktor::ui
{
	namespace
	{

// ============================================================
// Popup listeners (decoration-less centered dialog)
// ============================================================

void dialogPopupConfigure(void* data, struct xdg_popup* popup, int32_t x, int32_t y, int32_t width, int32_t height)
{
}

void dialogPopupDone(void* data, struct xdg_popup* popup)
{
	DialogWl* dialog = static_cast< DialogWl* >(data);
	dialog->endModal(DialogResult::Cancel);
}

static const struct xdg_popup_listener s_dialogPopupListener = {
	dialogPopupConfigure,
	dialogPopupDone
};

// ============================================================
// SSD path
// ============================================================

void ssdDialogXdgSurfaceConfigure(void* data, struct xdg_surface* xdgSurface, uint32_t serial)
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

static const struct xdg_surface_listener s_ssdDialogXdgSurfaceListener = {
	ssdDialogXdgSurfaceConfigure
};

void ssdDialogXdgToplevelConfigure(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states)
{
	DialogWl* dialog = static_cast< DialogWl* >(data);
	WidgetData* wd = static_cast< WidgetData* >(dialog->getInternalHandle());
	wd->pendingWidth = width;
	wd->pendingHeight = height;
}

void ssdDialogXdgToplevelClose(void* data, struct xdg_toplevel* toplevel)
{
	DialogWl* dialog = static_cast< DialogWl* >(data);
	WidgetData* wd = static_cast< WidgetData* >(dialog->getInternalHandle());

	WlEvent e;
	e.type = WlEvtClose;
	e.surface = wd->surface;
	dialog->getContextWl()->dispatch(e);
}

static const struct xdg_toplevel_listener s_ssdDialogXdgToplevelListener = {
	ssdDialogXdgToplevelConfigure,
	ssdDialogXdgToplevelClose
};

// ============================================================
// libdecor fallback path
// ============================================================

void libdecorDialogConfigure(struct libdecor_frame* frame, struct libdecor_configuration* configuration, void* userData)
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

void libdecorDialogClose(struct libdecor_frame* frame, void* userData)
{
	DialogWl* dialog = static_cast< DialogWl* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(dialog->getInternalHandle());

	WlEvent e;
	e.type = WlEvtClose;
	e.surface = wd->surface;
	dialog->getContextWl()->dispatch(e);
}

void libdecorDialogCommit(struct libdecor_frame* frame, void* userData)
{
	DialogWl* dialog = static_cast< DialogWl* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(dialog->getInternalHandle());
	wl_surface_commit(wd->surface);
}

void libdecorDialogDismissPopup(struct libdecor_frame* frame, const char* seatName, void* userData)
{
}

static struct libdecor_frame_interface s_libdecorDialogFrameInterface = {
	libdecorDialogConfigure,
	libdecorDialogClose,
	libdecorDialogCommit,
	libdecorDialogDismissPopup,
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

	const bool wantDecorations = (style & (WsResizable | WsCaption | WsSystemBox | WsMinimizeBox | WsMaximizeBox)) != 0;

	if (!wantDecorations)
	{
		// Decoration-less dialog (e.g. splash screen).
		m_data.xdgSurface = xdg_wm_base_get_xdg_surface(m_context->getXdgWmBase(), m_data.surface);
		xdg_surface_add_listener(m_data.xdgSurface, &s_ssdDialogXdgSurfaceListener, this);

		// If centering is requested and we have a parent with an xdg_surface,
		// use an xdg_popup with a centered positioner — this is the only way
		// to position a toplevel on Wayland.
		WidgetData* parentData = (parent != nullptr) ? static_cast< WidgetData* >(parent->getInternalHandle()) : nullptr;
		WidgetData* anchorData = nullptr;
		if (parentData)
		{
			for (WidgetData* w = parentData; w != nullptr; w = w->parent)
			{
				if (w->xdgSurface) { anchorData = w; break; }
			}
		}

		if ((style & Dialog::WsCenterDesktop) != 0 && anchorData)
		{
			// Compute centered anchor position in the parent surface (logical coords).
			const int32_t scale = m_context->getOutputScale();
			const int32_t pw = m_context->getOutputWidth() / scale;
			const int32_t ph = m_context->getOutputHeight() / scale;
			const int32_t dw = m_rect.getWidth() / scale;
			const int32_t dh = m_rect.getHeight() / scale;
			const int32_t cx = (pw - dw) / 2;
			const int32_t cy = (ph - dh) / 2;

			struct xdg_positioner* positioner = xdg_wm_base_create_positioner(m_context->getXdgWmBase());
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
			// Non-centered decoration-less toplevel.
			m_data.xdgToplevel = xdg_surface_get_toplevel(m_data.xdgSurface);
			xdg_toplevel_add_listener(m_data.xdgToplevel, &s_ssdDialogXdgToplevelListener, this);

			if (anchorData && anchorData->xdgToplevel)
				xdg_toplevel_set_parent(m_data.xdgToplevel, anchorData->xdgToplevel);
		}

		wl_surface_commit(m_data.surface);
		wl_display_roundtrip(m_context->getDisplay());
	}
	else if (m_context->hasServerSideDecorations())
	{
		// SSD path.
		m_data.xdgSurface = xdg_wm_base_get_xdg_surface(m_context->getXdgWmBase(), m_data.surface);
		xdg_surface_add_listener(m_data.xdgSurface, &s_ssdDialogXdgSurfaceListener, this);

		m_data.xdgToplevel = xdg_surface_get_toplevel(m_data.xdgSurface);
		xdg_toplevel_add_listener(m_data.xdgToplevel, &s_ssdDialogXdgToplevelListener, this);

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
			&s_libdecorDialogFrameInterface,
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

	m_context->bind(&m_data, WlEvtClose, [this](WlEvent& e) {
		CloseEvent closeEvent(m_owner);
		m_owner->raiseEvent(&closeEvent);
		if (!(closeEvent.consumed() && closeEvent.cancelled()))
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
	else if (m_data.xdgToplevel)
	{
		const int32_t scale = m_context->getOutputScale();
		xdg_toplevel_set_min_size(m_data.xdgToplevel, minSize.cx / scale, minSize.cy / scale);
	}
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
