/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include "Core/RefArray.h"
#include "Input/IInputDriver.h"
#include "Input/Linux/pointer-constraints-unstable-v1-client-protocol.h"
#include "Input/Linux/relative-pointer-unstable-v1-client-protocol.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_LINUX_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

class IInputDevice;
class MouseDeviceLinux;
class KeyboardDeviceLinux;

class T_DLLCLASS InputDriverLinux : public IInputDriver
{
	T_RTTI_CLASS;

public:
	virtual ~InputDriverLinux();

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, InputCategory inputCategories) override;

	virtual int getDeviceCount() override;

	virtual Ref< IInputDevice > getDevice(int index) override;

	virtual UpdateResult update() override;

	virtual void setSize(int32_t width, int32_t height) override;

private:
	// Wayland connection (display + surface are borrowed from the renderer).
	wl_display* m_display = nullptr;
	wl_surface* m_surface = nullptr;
	wl_event_queue* m_eventQueue = nullptr;
	wl_registry* m_registry = nullptr;
	wl_seat* m_seat = nullptr;
	wl_pointer* m_pointer = nullptr;
	wl_keyboard* m_keyboard = nullptr;

	// Relative motion / pointer lock for exclusive (camera) mouse.
	zwp_relative_pointer_manager_v1* m_relativePointerManager = nullptr;
	zwp_relative_pointer_v1* m_relativePointer = nullptr;
	zwp_pointer_constraints_v1* m_pointerConstraints = nullptr;
	zwp_locked_pointer_v1* m_lockedPointer = nullptr;
	bool m_pointerLocked = false;

	// Keyboard translation.
	xkb_context* m_xkbContext = nullptr;
	xkb_keymap* m_xkbKeymap = nullptr;
	xkb_state* m_xkbState = nullptr;

	Ref< MouseDeviceLinux > m_mouse;
	Ref< KeyboardDeviceLinux > m_keyboardDevice;
	RefArray< IInputDevice > m_devices;

	void setupWayland(const SystemWindow& syswin);
	void pumpWayland();
	void updatePointerLock();

public:
	// Wayland C-style listener callbacks (public so the file-scope listener
	// tables can reference them; each receives an InputDriverLinux* as data).

	// Registry listener.
	static void registryGlobal(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
	static void registryGlobalRemove(void* data, wl_registry* registry, uint32_t name);

	// Seat listener.
	static void seatCapabilities(void* data, wl_seat* seat, uint32_t caps);
	static void seatName(void* data, wl_seat* seat, const char* name);

	// Pointer listener.
	static void pointerEnter(void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy);
	static void pointerLeave(void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface);
	static void pointerMotion(void* data, wl_pointer* pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
	static void pointerButton(void* data, wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
	static void pointerAxis(void* data, wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
	static void pointerFrame(void* data, wl_pointer* pointer);
	static void pointerAxisSource(void* data, wl_pointer* pointer, uint32_t source);
	static void pointerAxisStop(void* data, wl_pointer* pointer, uint32_t time, uint32_t axis);
	static void pointerAxisDiscrete(void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete);

	// Keyboard listener.
	static void keyboardKeymap(void* data, wl_keyboard* keyboard, uint32_t format, int32_t fd, uint32_t size);
	static void keyboardEnter(void* data, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface, wl_array* keys);
	static void keyboardLeave(void* data, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface);
	static void keyboardKey(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
	static void keyboardModifiers(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t modsDepressed, uint32_t modsLatched, uint32_t modsLocked, uint32_t group);
	static void keyboardRepeatInfo(void* data, wl_keyboard* keyboard, int32_t rate, int32_t delay);

	// Relative pointer listener.
	static void relativePointerMotion(void* data, zwp_relative_pointer_v1* rp, uint32_t utimeHi, uint32_t utimeLo, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dxUnaccel, wl_fixed_t dyUnaccel);

	// Locked pointer listener.
	static void lockedPointerLocked(void* data, zwp_locked_pointer_v1* lp);
	static void lockedPointerUnlocked(void* data, zwp_locked_pointer_v1* lp);
};

}
