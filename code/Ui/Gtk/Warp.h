#pragma once

#include <gtk/gtk.h>

namespace traktor
{
	namespace ui
	{

struct Warp
{
	GtkWidget* widget;			//!< This widget.

	Warp()
	:	widget(nullptr)
	{
	}
};

	}
}
