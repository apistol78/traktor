/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <libdecor.h>
#include <wayland-client.h>
#include "Ui/Itf/IWidget.h"
#include "Ui/Wl/ContextWl.h"
#include "Ui/Wl/TypesWl.h"
#include "xdg-shell-client-protocol.h"

namespace traktor::ui
{

/*! Userdata bridging C-style Wayland callbacks to the C++ widget interface.
 *  Each toplevel owns a ToplevelListenerCtx and passes its address as the
 *  userdata pointer for shared xdg_surface / xdg_toplevel / libdecor listeners.
 *  Sharing the listeners avoids the per-toplevel boilerplate that would
 *  otherwise be repeated in Form/Dialog/ToolForm.
 */
struct ToplevelListenerCtx
{
	ContextWl* context = nullptr;
	IWidget* widget = nullptr;
};

inline void applyToplevelConfiguredSize(ContextWl* context, IWidget* widget, int32_t logicalWidth, int32_t logicalHeight)
{
	if (logicalWidth <= 0 || logicalHeight <= 0)
		return;

	// Remember the logical size so we can re-derive the device rect if the
	// fractional scale changes later (e.g. the window moves to another output).
	WidgetData* wd = static_cast< WidgetData* >(widget->getInternalHandle());
	wd->logicalWidth = logicalWidth;
	wd->logicalHeight = logicalHeight;

	const Rect prev = widget->getRect();
	const Rect next(prev.left, prev.top, prev.left + context->toDevice(logicalWidth), prev.top + context->toDevice(logicalHeight));

	// setRect queues an expose; the event loop drains it at the end of the
	// current iteration. Draining synchronously here would force a full
	// repaint (and a Vulkan swapchain rebuild for any embedded 3D view) for
	// every configure event the compositor sends during an interactive drag.
	widget->setRect(next);
}

inline void toplevelXdgSurfaceConfigure(void* data, xdg_surface* xdgSurface, uint32_t serial)
{
	xdg_surface_ack_configure(xdgSurface, serial);

	auto* lctx = static_cast< ToplevelListenerCtx* >(data);
	WidgetData* wd = static_cast< WidgetData* >(lctx->widget->getInternalHandle());
	wd->configured = true;

	if (wd->pendingWidth > 0 && wd->pendingHeight > 0)
	{
		const int32_t pw = wd->pendingWidth;
		const int32_t ph = wd->pendingHeight;
		wd->pendingWidth = 0;
		wd->pendingHeight = 0;
		applyToplevelConfiguredSize(lctx->context, lctx->widget, pw, ph);
	}
}

inline void toplevelXdgToplevelConfigure(void* data, xdg_toplevel*, int32_t width, int32_t height, wl_array*)
{
	auto* lctx = static_cast< ToplevelListenerCtx* >(data);
	WidgetData* wd = static_cast< WidgetData* >(lctx->widget->getInternalHandle());
	wd->pendingWidth = width;
	wd->pendingHeight = height;
}

inline void toplevelXdgToplevelClose(void* data, xdg_toplevel*)
{
	auto* lctx = static_cast< ToplevelListenerCtx* >(data);
	WidgetData* wd = static_cast< WidgetData* >(lctx->widget->getInternalHandle());
	WlEvent e;
	e.type = WlEvtClose;
	e.surface = wd->surface;
	lctx->context->enqueueEvent(e);
}

inline void toplevelLibdecorConfigure(libdecor_frame* frame, libdecor_configuration* configuration, void* userData)
{
	auto* lctx = static_cast< ToplevelListenerCtx* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(lctx->widget->getInternalHandle());

	int width = 0, height = 0;
	if (!libdecor_configuration_get_content_size(configuration, frame, &width, &height))
	{
		const Rect rc = lctx->widget->getRect();
		width = lctx->context->toLogical(rc.getWidth());
		height = lctx->context->toLogical(rc.getHeight());
	}

	libdecor_state* state = libdecor_state_new(width, height);
	libdecor_frame_commit(frame, state, configuration);
	libdecor_state_free(state);

	wd->configured = true;
	applyToplevelConfiguredSize(lctx->context, lctx->widget, width, height);
}

inline void toplevelLibdecorClose(libdecor_frame*, void* userData)
{
	auto* lctx = static_cast< ToplevelListenerCtx* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(lctx->widget->getInternalHandle());
	WlEvent e;
	e.type = WlEvtClose;
	e.surface = wd->surface;
	lctx->context->enqueueEvent(e);
}

inline void toplevelLibdecorCommit(libdecor_frame*, void* userData)
{
	auto* lctx = static_cast< ToplevelListenerCtx* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(lctx->widget->getInternalHandle());
	wl_surface_commit(wd->surface);
}

inline void toplevelLibdecorDismissPopup(libdecor_frame*, const char*, void*)
{
}

//! Compositor-preferred fractional scale changed (units of 1/120). Update the
//! context's effective scale/DPI and, if the size is already known, re-derive the
//! device rect at the new scale — setRect drives the relayout + redraw, which
//! re-commits buffers at buffer_scale=1 and updates the toplevel viewport.
inline void toplevelFractionalPreferredScale(void* data, wp_fractional_scale_v1*, uint32_t scale)
{
	auto* lctx = static_cast< ToplevelListenerCtx* >(data);
	if (!lctx->context->applyFractionalScale(scale))
		return;

	WidgetData* wd = static_cast< WidgetData* >(lctx->widget->getInternalHandle());
	if (wd->configured && wd->logicalWidth > 0 && wd->logicalHeight > 0)
		applyToplevelConfiguredSize(lctx->context, lctx->widget, wd->logicalWidth, wd->logicalHeight);
}

inline const wp_fractional_scale_v1_listener s_toplevelFractionalScaleListener = {
	toplevelFractionalPreferredScale
};

//! Attach a wp_fractional_scale_v1 to a toplevel surface if the compositor supports
//! the protocol. No-op otherwise (legacy integer buffer_scale path stays in effect).
inline void toplevelSetupFractionalScale(ToplevelListenerCtx* lctx, WidgetData* wd)
{
	wp_fractional_scale_manager_v1* manager = lctx->context->getFractionalScaleManager();
	// Fractional scaling is done via buffer_scale=1 + wp_viewport, so both protocols
	// must be present; without the viewporter, stay on the legacy integer path.
	if (manager == nullptr || lctx->context->getViewporter() == nullptr || wd->surface == nullptr || wd->fractionalScale != nullptr)
		return;

	wd->fractionalScale = wp_fractional_scale_manager_v1_get_fractional_scale(manager, wd->surface);
	wp_fractional_scale_v1_add_listener(wd->fractionalScale, &s_toplevelFractionalScaleListener, lctx);
}

inline const xdg_surface_listener s_toplevelXdgSurfaceListener = {
	toplevelXdgSurfaceConfigure
};

inline const xdg_toplevel_listener s_toplevelXdgToplevelListener = {
	toplevelXdgToplevelConfigure,
	toplevelXdgToplevelClose
};

inline libdecor_frame_interface s_toplevelLibdecorFrameInterface = {
	toplevelLibdecorConfigure,
	toplevelLibdecorClose,
	toplevelLibdecorCommit,
	toplevelLibdecorDismissPopup,
	nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr
};

}
