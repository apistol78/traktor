/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

uint32_t cglwGetDisplayModeCount();

bool cglwGetDisplayMode(uint32_t index, DisplayMode& outDisplayMode);

bool cglwGetCurrentDisplayMode(DisplayMode& outDisplayMode);

void* cglwCreateWindow(const std::wstring& title, const DisplayMode& displayMode, bool fullscreen);

void cglwDestroyWindow(void* windowHandle);

bool cglwModifyWindow(void* windowHandle, const DisplayMode& displayMode, bool fullscreen);

bool cglwIsFullscreen(void* windowHandle);

bool cglwIsActive(void* windowHandle);

void cglwSetCursorVisible(void* windowHandle, bool visible);

bool cglwUpdateWindow(void* windowHandle, RenderEvent& outEvent);

void* cglwGetWindowView(void* windowHandle);

	}
}

