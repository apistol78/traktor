#pragma once

#include <X11/Xlib.h>

namespace traktor
{
	namespace ui
	{

struct WidgetData
{
	WidgetData* parent;
	Window window;
	bool topLevel;
	bool visible;
	bool enable;
	bool grabbed;

	WidgetData()
	:	parent(nullptr)
	,	window(None)
	,	topLevel(false)
	,	visible(false)
	,	enable(true)
	,	grabbed(false)
	{
	}
};

	}
}