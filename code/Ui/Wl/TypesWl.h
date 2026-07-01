/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <wayland-client.h>
#include "Core/Containers/AlignedVector.h"
#include "xdg-shell-client-protocol.h"
#include "viewporter-client-protocol.h"
#include "fractional-scale-v1-client-protocol.h"

struct xdg_surface;
struct xdg_toplevel;
struct libdecor_frame;
struct zxdg_toplevel_decoration_v1;

namespace traktor::ui
{

struct WidgetData
{
	WidgetData* parent = nullptr;
	AlignedVector< WidgetData* > children;		//!< Direct children; maintained so a parent resize can propagate clip recomputation.
	wl_surface* surface = nullptr;
	wl_subsurface* subsurface = nullptr;
	wp_viewport* viewport = nullptr;			//!< Crops m_data.surface to the visible region within the ancestor chain. For toplevels (fractional mode) this instead scales the whole buffer to logical size.
	wp_fractional_scale_v1* fractionalScale = nullptr;	//!< Toplevel only: receives the compositor's preferred fractional scale.
	wl_surface* renderSurface = nullptr;		//!< Dedicated child surface for Vulkan/GL rendering.
	wl_subsurface* renderSubsurface = nullptr;
	wp_viewport* renderViewport = nullptr;		//!< Crops the render surface to the same visible region as the parent widget.
	xdg_surface* xdgSurface = nullptr;
	xdg_toplevel* xdgToplevel = nullptr;
	xdg_popup* xdgPopup = nullptr;
	libdecor_frame* frame = nullptr;
	zxdg_toplevel_decoration_v1* decoration = nullptr;
	bool topLevel = false;
	bool mapped = false;
	bool visible = false;
	bool enable = true;
	bool grabbed = false;
	bool configured = false;
	int32_t posX = 0;			//!< Widget position (device coords), kept in sync with m_rect for parent chain walks.
	int32_t posY = 0;
	int32_t width = 0;			//!< Widget size (device coords), needed by descendants for ancestor-chain clip.
	int32_t height = 0;
	int32_t pendingWidth = 0;
	int32_t pendingHeight = 0;
	int32_t logicalWidth = 0;	//!< Toplevel only: last configured size in logical pixels, re-applied when the fractional scale changes.
	int32_t logicalHeight = 0;
};

}
