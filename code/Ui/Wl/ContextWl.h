/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>
#include <libdecor.h>
#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Timer/Timer.h"
#include "Ui/Wl/TypesWl.h"

struct xdg_wm_base;
struct zxdg_decoration_manager_v1;

namespace traktor::ui
{

struct WidgetData;

/*! Wayland event types (internal). */
enum WlEventType
{
	WlEvtPointerEnter = 1,
	WlEvtPointerLeave,
	WlEvtPointerMotion,
	WlEvtPointerButton,
	WlEvtPointerAxis,
	WlEvtKeyboardEnter,
	WlEvtKeyboardLeave,
	WlEvtKeyboardKey,
	WlEvtKeyboardModifiers,
	WlEvtConfigure,
	WlEvtClose,
	WlEvtExpose,
	WlEvtFocusIn,
	WlEvtFocusOut
};

/*! Wayland event data. */
struct WlEvent
{
	int32_t type = 0;
	wl_surface* surface = nullptr;
	double stamp = 0.0;
	// Pointer
	double pointerX = 0.0;
	double pointerY = 0.0;
	uint32_t button = 0;
	uint32_t buttonState = 0;
	int32_t buttons = 0;		//!< Currently held button bitmask (MbtLeft|MbtMiddle|MbtRight).
	double axisValue = 0.0;
	uint32_t axisType = 0;
	// Keyboard
	uint32_t key = 0;
	uint32_t keyState = 0;
	uint32_t modifiers = 0;
	uint32_t serial = 0;
	// Configure
	int32_t width = 0;
	int32_t height = 0;
};

class ContextWl : public Object
{
	T_RTTI_CLASS;

public:
	explicit ContextWl();

	virtual ~ContextWl();

	bool initialize();

	//! Bind callback for specified event with given widget target.
	void bind(WidgetData* widget, int32_t eventType, const std::function< void(WlEvent& e) >& fn);

	//! Unbind all callbacks for specified widget.
	void unbind(WidgetData* widget);

	//! Register a widget's render surface as an input alias so pointer
	//! events on the render surface are routed to the owning widget.
	void bindRenderSurface(WidgetData* widget);

	//! Push a popup onto the active popup stack (called from createPopup).
	void pushPopup(WidgetData* widget);

	//! Destroy all popups above (and including) the given widget,
	//! enforcing xdg-shell LIFO destruction order.
	void destroyPopupsAbove(WidgetData* widget);

	//! Push modal widget.
	void pushModal(WidgetData* widget);

	//! Pop modal widget.
	void popModal(WidgetData* widget);

	//! Grab input to widget.
	void grab(WidgetData* widget);

	//! Ungrab input.
	void ungrab(WidgetData* widget);

	//! Dispatch event to callbacks.
	void dispatch(WlEvent& e);

	//! Push event into deferred queue (called from Wayland listener callbacks).
	void enqueueEvent(const WlEvent& e);

	//! Process all queued events, raising global events on owner first.
	//! Returns true if any event was processed.
	bool processEventQueue(EventSubject* owner);

	//! Create shared memory buffer.
	struct wl_buffer* createShmBuffer(int32_t width, int32_t height, int32_t stride, void** outData);

	//@{

	wl_display* getDisplay() const { return m_display; }

	wl_compositor* getCompositor() const { return m_compositor; }

	wl_subcompositor* getSubcompositor() const { return m_subcompositor; }

	wl_shm* getShm() const { return m_shm; }

	xdg_wm_base* getXdgWmBase() const { return m_xdgWmBase; }

	wl_seat* getSeat() const { return m_seat; }

	wl_pointer* getPointer() const { return m_pointer; }

	wl_cursor_theme* getCursorTheme() const { return m_cursorTheme; }

	wl_surface* getCursorSurface() const { return m_cursorSurface; }

	libdecor* getLibdecor() const { return m_libdecor; }

	//! True if the compositor supports server-side decorations via zxdg_decoration_manager_v1.
	bool hasServerSideDecorations() const { return m_decorationManager != nullptr; }

	zxdg_decoration_manager_v1* getDecorationManager() const { return m_decorationManager; }

	xkb_state* getXkbState() const { return m_xkbState; }

	bool anyGrabbed() const { return m_grabbed != nullptr; }

	WidgetData* getPointerFocus() const { return m_pointerFocus; }

	uint32_t getPointerSerial() const { return m_pointerSerial; }

	//! Serial from the most recent button press (for xdg_popup_grab).
	uint32_t getGrabSerial() const { return m_grabSerial; }

	uint32_t getKeyboardModifiers() const { return m_keyboardModifiers; }

	int32_t getOutputScale() const { return m_outputScale; }

	//@}

	//! Set internal focus to a widget within the toplevel.
	//! Sends FocusOut to the previously focused widget and FocusIn to the new one.
	void setInternalFocus(WidgetData* widget);

	//! Get the internally focused widget (for keyboard event routing).
	WidgetData* getInternalFocus() const { return m_internalFocus; }

	//! Queue a widget for deferred redraw (processed in event loop).
	void queueExpose(WidgetData* widget);

	//! Process all pending expose requests; returns true if any were processed.
	bool processPendingExposes();

	//@{

	int32_t getSystemDPI() const;

	//@}

private:
	struct Binding
	{
		WidgetData* widget = nullptr;
		SmallMap< int32_t, std::function< void(WlEvent& e) > > fns;
	};

	wl_display* m_display = nullptr;
	wl_registry* m_registry = nullptr;
	wl_compositor* m_compositor = nullptr;
	wl_subcompositor* m_subcompositor = nullptr;
	wl_shm* m_shm = nullptr;
	wl_seat* m_seat = nullptr;
	wl_pointer* m_pointer = nullptr;
	wl_keyboard* m_keyboard = nullptr;
	wl_cursor_theme* m_cursorTheme = nullptr;
	wl_surface* m_cursorSurface = nullptr;
	xdg_wm_base* m_xdgWmBase = nullptr;
	libdecor* m_libdecor = nullptr;
	zxdg_decoration_manager_v1* m_decorationManager = nullptr;
	wl_output* m_output = nullptr;

	xkb_context* m_xkbContext = nullptr;
	xkb_keymap* m_xkbKeymap = nullptr;
	xkb_state* m_xkbState = nullptr;

	int32_t m_dpi = 96;
	int32_t m_outputScale = 1;
	uint32_t m_pointerSerial = 0;
	uint32_t m_grabSerial = 0;
	uint32_t m_keyboardModifiers = 0;
	int32_t m_repeatRate = 0;		//!< Key repeats per second (0 = disabled).
	int32_t m_repeatDelay = 0;		//!< Delay in ms before first repeat.
	int32_t m_repeatTimer = -1;		//!< Timers handle for active repeat, or -1.
	uint32_t m_repeatKey = 0;		//!< Key code currently repeating.
	double m_pointerX = 0.0;
	double m_pointerY = 0.0;
	int32_t m_buttonMask = 0;		//!< Currently pressed buttons (MbtLeft | MbtMiddle | MbtRight).

	SmallMap< wl_surface*, Binding > m_bindings;
	AlignedVector< WidgetData* > m_modal;
	WidgetData* m_grabbed = nullptr;
	WidgetData* m_pointerFocus = nullptr;
	WidgetData* m_keyboardFocus = nullptr;
	WidgetData* m_internalFocus = nullptr;
	AlignedVector< WidgetData* > m_popupStack;	//!< Active xdg_popups in creation order.
	AlignedVector< WidgetData* > m_pendingExposes;
	AlignedVector< WlEvent > m_eventQueue;

	static Timer ms_timer;

	void dispatch(wl_surface* surface, int32_t eventType, bool always, WlEvent& e);
	bool preTranslateEvent(EventSubject* owner, WlEvent& e);

public:
	// Registry listener
	static void registryGlobal(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
	static void registryGlobalRemove(void* data, wl_registry* registry, uint32_t name);

	// Seat listener
	static void seatCapabilities(void* data, wl_seat* seat, uint32_t caps);
	static void seatName(void* data, wl_seat* seat, const char* name);

	// Pointer listener
	static void pointerEnter(void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy);
	static void pointerLeave(void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface);
	static void pointerMotion(void* data, wl_pointer* pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
	static void pointerButton(void* data, wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
	static void pointerAxis(void* data, wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
	static void pointerFrame(void* data, wl_pointer* pointer);
	static void pointerAxisSource(void* data, wl_pointer* pointer, uint32_t source);
	static void pointerAxisStop(void* data, wl_pointer* pointer, uint32_t time, uint32_t axis);
	static void pointerAxisDiscrete(void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete);

	// Keyboard listener
	static void keyboardKeymap(void* data, wl_keyboard* keyboard, uint32_t format, int32_t fd, uint32_t size);
	static void keyboardEnter(void* data, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface, wl_array* keys);
	static void keyboardLeave(void* data, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface);
	static void keyboardKey(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
	static void keyboardModifiers(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t modsDepressed, uint32_t modsLatched, uint32_t modsLocked, uint32_t group);
	static void keyboardRepeatInfo(void* data, wl_keyboard* keyboard, int32_t rate, int32_t delay);

	// XDG WM base listener
	static void xdgWmBasePing(void* data, xdg_wm_base* wmBase, uint32_t serial);

	// Output listener
	static void outputGeometry(void* data, wl_output* output, int32_t x, int32_t y, int32_t pw, int32_t ph, int32_t subpixel, const char* make, const char* model, int32_t transform);
	static void outputMode(void* data, wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);
	static void outputDone(void* data, wl_output* output);
	static void outputScale(void* data, wl_output* output, int32_t factor);
};

}
