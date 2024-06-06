/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Ui/X11/TypesX11.h"

namespace traktor::ui
{

struct WidgetData;

class Context : public Object
{
	T_RTTI_CLASS;

public:
	explicit Context(Display* display, int screen, XIM xim);

	//! Bind callback for specified event with given window target.
	void bind(WidgetData* widget, int32_t eventType, const std::function< void(XEvent& xe) >& fn);

	//! Unbind all callbacks for specified window.
	void unbind(WidgetData* widget);

	//! Push modal widget.
	void pushModal(WidgetData* widget);

	//! Pop modal widget.
	void popModal(WidgetData* widget);

	//! Grab input to widget.
	void grab(WidgetData* widget);

	//! Ungrab input.
	void ungrab(WidgetData* widget);

	//! Dispatch event to callbacks.
	void dispatch(XEvent& xe);

	//@{

	Display* getDisplay() const { return m_display; }

	int getScreen() const { return m_screen; }

	Window getRootWindow() const;

	XIM getXIM() const { return m_xim; }

	bool anyGrabbed() const { return m_grabbed != nullptr; }

	//@}

	//@{

	int32_t getSystemDPI() const;

	//@}

private:
	struct Binding
	{
		WidgetData* widget = nullptr;
		SmallMap< int32_t, std::function< void(XEvent& xe) > > fns;
	};

	Display* m_display = nullptr;
	int m_screen = 0;
	XIM m_xim;
	int32_t m_dpi = 0;
	SmallMap< Window, Binding > m_bindings;
	AlignedVector< WidgetData* > m_modal;
	WidgetData* m_grabbed = nullptr;

	void dispatch(Window window, int32_t eventType, bool always, XEvent& xe);
};

}
