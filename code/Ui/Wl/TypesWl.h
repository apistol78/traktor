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
#include "xdg-shell-client-protocol.h"

struct xdg_surface;
struct xdg_toplevel;
struct libdecor_frame;
struct zxdg_toplevel_decoration_v1;

namespace traktor::ui
{

struct WidgetData
{
	WidgetData* parent = nullptr;
	wl_surface* surface = nullptr;
	wl_subsurface* subsurface = nullptr;
	wl_surface* renderSurface = nullptr;		//!< Dedicated child surface for Vulkan/GL rendering.
	wl_subsurface* renderSubsurface = nullptr;
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
	int32_t pendingWidth = 0;
	int32_t pendingHeight = 0;
};

}
