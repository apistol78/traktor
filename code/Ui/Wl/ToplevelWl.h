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

	const int32_t scale = context->getOutputScale();
	const Rect prev = widget->getRect();
	const Rect next(prev.left, prev.top, prev.left + logicalWidth * scale, prev.top + logicalHeight * scale);

	widget->setRect(next);
	context->processPendingExposes();
	widget->update(nullptr, true);
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
	lctx->context->dispatch(e);
}

inline void toplevelLibdecorConfigure(libdecor_frame* frame, libdecor_configuration* configuration, void* userData)
{
	auto* lctx = static_cast< ToplevelListenerCtx* >(userData);
	WidgetData* wd = static_cast< WidgetData* >(lctx->widget->getInternalHandle());

	int width = 0, height = 0;
	if (!libdecor_configuration_get_content_size(configuration, frame, &width, &height))
	{
		const Rect rc = lctx->widget->getRect();
		const int32_t scale = lctx->context->getOutputScale();
		width = rc.getWidth() / scale;
		height = rc.getHeight() / scale;
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
	lctx->context->dispatch(e);
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
