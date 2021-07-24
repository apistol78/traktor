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
