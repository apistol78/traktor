/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
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
#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
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
	struct wl_surface* surface = nullptr;
	// Pointer
	double pointerX = 0.0;
	double pointerY = 0.0;
	uint32_t button = 0;
	uint32_t buttonState = 0;
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

	struct wl_display* getDisplay() const { return m_display; }

	struct wl_compositor* getCompositor() const { return m_compositor; }

	struct wl_subcompositor* getSubcompositor() const { return m_subcompositor; }

	struct wl_shm* getShm() const { return m_shm; }

	struct xdg_wm_base* getXdgWmBase() const { return m_xdgWmBase; }

	struct wl_seat* getSeat() const { return m_seat; }

	struct wl_pointer* getPointer() const { return m_pointer; }

	struct wl_cursor_theme* getCursorTheme() const { return m_cursorTheme; }

	struct wl_surface* getCursorSurface() const { return m_cursorSurface; }

	struct zxdg_decoration_manager_v1* getDecorationManager() const { return m_decorationManager; }

	struct xkb_state* getXkbState() const { return m_xkbState; }

	bool anyGrabbed() const { return m_grabbed != nullptr; }

	WidgetData* getPointerFocus() const { return m_pointerFocus; }

	uint32_t getPointerSerial() const { return m_pointerSerial; }

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

	struct wl_display* m_display = nullptr;
	struct wl_registry* m_registry = nullptr;
	struct wl_compositor* m_compositor = nullptr;
	struct wl_subcompositor* m_subcompositor = nullptr;
	struct wl_shm* m_shm = nullptr;
	struct wl_seat* m_seat = nullptr;
	struct wl_pointer* m_pointer = nullptr;
	struct wl_keyboard* m_keyboard = nullptr;
	struct wl_cursor_theme* m_cursorTheme = nullptr;
	struct wl_surface* m_cursorSurface = nullptr;
	struct xdg_wm_base* m_xdgWmBase = nullptr;
	struct zxdg_decoration_manager_v1* m_decorationManager = nullptr;
	struct wl_output* m_output = nullptr;

	struct xkb_context* m_xkbContext = nullptr;
	struct xkb_keymap* m_xkbKeymap = nullptr;
	struct xkb_state* m_xkbState = nullptr;

	int32_t m_dpi = 96;
	int32_t m_outputScale = 1;
	uint32_t m_pointerSerial = 0;
	uint32_t m_keyboardModifiers = 0;
	double m_pointerX = 0.0;
	double m_pointerY = 0.0;

	SmallMap< struct wl_surface*, Binding > m_bindings;
	AlignedVector< WidgetData* > m_modal;
	WidgetData* m_grabbed = nullptr;
	WidgetData* m_pointerFocus = nullptr;
	WidgetData* m_keyboardFocus = nullptr;
	WidgetData* m_internalFocus = nullptr;
	AlignedVector< WidgetData* > m_pendingExposes;
	AlignedVector< WlEvent > m_eventQueue;

	void dispatch(struct wl_surface* surface, int32_t eventType, bool always, WlEvent& e);
	bool preTranslateEvent(EventSubject* owner, WlEvent& e);

public:
	// Registry listener
	static void registryGlobal(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
	static void registryGlobalRemove(void* data, struct wl_registry* registry, uint32_t name);

	// Seat listener
	static void seatCapabilities(void* data, struct wl_seat* seat, uint32_t caps);
	static void seatName(void* data, struct wl_seat* seat, const char* name);

	// Pointer listener
	static void pointerEnter(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy);
	static void pointerLeave(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface);
	static void pointerMotion(void* data, struct wl_pointer* pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
	static void pointerButton(void* data, struct wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
	static void pointerAxis(void* data, struct wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
	static void pointerFrame(void* data, struct wl_pointer* pointer);
	static void pointerAxisSource(void* data, struct wl_pointer* pointer, uint32_t source);
	static void pointerAxisStop(void* data, struct wl_pointer* pointer, uint32_t time, uint32_t axis);
	static void pointerAxisDiscrete(void* data, struct wl_pointer* pointer, uint32_t axis, int32_t discrete);

	// Keyboard listener
	static void keyboardKeymap(void* data, struct wl_keyboard* keyboard, uint32_t format, int32_t fd, uint32_t size);
	static void keyboardEnter(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys);
	static void keyboardLeave(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface);
	static void keyboardKey(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
	static void keyboardModifiers(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t modsDepressed, uint32_t modsLatched, uint32_t modsLocked, uint32_t group);
	static void keyboardRepeatInfo(void* data, struct wl_keyboard* keyboard, int32_t rate, int32_t delay);

	// XDG WM base listener
	static void xdgWmBasePing(void* data, struct xdg_wm_base* wmBase, uint32_t serial);

	// Output listener
	static void outputGeometry(void* data, struct wl_output* output, int32_t x, int32_t y, int32_t pw, int32_t ph, int32_t subpixel, const char* make, const char* model, int32_t transform);
	static void outputMode(void* data, struct wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);
	static void outputDone(void* data, struct wl_output* output);
	static void outputScale(void* data, struct wl_output* output, int32_t factor);
};

}
