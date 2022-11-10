/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <X11/Xlib.h>

namespace traktor
{
	namespace ui
	{

struct WidgetData
{
	WidgetData* parent = nullptr;
	Window window = None;
	bool topLevel = false;
	bool mapped = false;
	bool visible = false;
	bool enable = true;
	bool grabbed = false;
	bool focus = false;
};

	}
}
