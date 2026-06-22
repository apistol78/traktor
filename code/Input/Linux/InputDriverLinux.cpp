/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Input/Linux/GamepadDeviceLinux.h"
#include "Input/Linux/InputDriverLinux.h"
#include "Input/Linux/KeyboardDeviceLinux.h"
#include "Input/Linux/MouseDeviceLinux.h"
#include "Input/Linux/TypesLinux.h"

namespace traktor::input
{
	namespace
	{

const wl_registry_listener c_registryListener =
{
	InputDriverLinux::registryGlobal,
	InputDriverLinux::registryGlobalRemove
};

const wl_seat_listener c_seatListener =
{
	InputDriverLinux::seatCapabilities,
	InputDriverLinux::seatName
};

const wl_pointer_listener c_pointerListener =
{
	InputDriverLinux::pointerEnter,
	InputDriverLinux::pointerLeave,
	InputDriverLinux::pointerMotion,
	InputDriverLinux::pointerButton,
	InputDriverLinux::pointerAxis,
	InputDriverLinux::pointerFrame,
	InputDriverLinux::pointerAxisSource,
	InputDriverLinux::pointerAxisStop,
	InputDriverLinux::pointerAxisDiscrete
};

const wl_keyboard_listener c_keyboardListener =
{
	InputDriverLinux::keyboardKeymap,
	InputDriverLinux::keyboardEnter,
	InputDriverLinux::keyboardLeave,
	InputDriverLinux::keyboardKey,
	InputDriverLinux::keyboardModifiers,
	InputDriverLinux::keyboardRepeatInfo
};

const zwp_relative_pointer_v1_listener c_relativePointerListener =
{
	InputDriverLinux::relativePointerMotion
};

const zwp_locked_pointer_v1_listener c_lockedPointerListener =
{
	InputDriverLinux::lockedPointerLocked,
	InputDriverLinux::lockedPointerUnlocked
};

//! Find the DefaultControl index whose keysym matches, or -1.
int32_t controlFromKeysym(xkb_keysym_t sym)
{
	if (sym == 0)
		return -1;
	for (uint32_t i = 0; i < sizeof_array(c_linuxControlKeys); ++i)
	{
		if (c_linuxControlKeys[i] == sym)
			return (int32_t)i;
	}
	return -1;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverLinux", 0, InputDriverLinux, IInputDriver)

InputDriverLinux::~InputDriverLinux()
{
	if (m_lockedPointer)
		zwp_locked_pointer_v1_destroy(m_lockedPointer);
	if (m_relativePointer)
		zwp_relative_pointer_v1_destroy(m_relativePointer);
	if (m_pointerConstraints)
		zwp_pointer_constraints_v1_destroy(m_pointerConstraints);
	if (m_relativePointerManager)
		zwp_relative_pointer_manager_v1_destroy(m_relativePointerManager);
	if (m_pointer)
		wl_pointer_destroy(m_pointer);
	if (m_keyboard)
		wl_keyboard_destroy(m_keyboard);
	if (m_seat)
		wl_seat_destroy(m_seat);
	if (m_registry)
		wl_registry_destroy(m_registry);
	if (m_xkbState)
		xkb_state_unref(m_xkbState);
	if (m_xkbKeymap)
		xkb_keymap_unref(m_xkbKeymap);
	if (m_xkbContext)
		xkb_context_unref(m_xkbContext);

	// Note: the wl_display is owned by the renderer and must not be disconnected.
	if (m_display && m_eventQueue)
		wl_display_flush(m_display);
	if (m_eventQueue)
		wl_event_queue_destroy(m_eventQueue);
}

bool InputDriverLinux::create(const SystemApplication& sysapp, const SystemWindow& syswin, InputCategory inputCategories)
{
	// Mouse and keyboard via the Wayland seat of the renderer's display.
	if (syswin.wl_display != nullptr)
	{
		setupWayland(syswin);

		m_mouse = new MouseDeviceLinux(syswin.width, syswin.height);
		m_keyboardDevice = new KeyboardDeviceLinux();

		m_devices.push_back(m_mouse);
		m_devices.push_back(m_keyboardDevice);

		log::info << L"Wayland mouse and keyboard input devices created." << Endl;
	}
	else
		log::warning << L"Input driver (Linux); no Wayland display in system window, mouse/keyboard unavailable." << Endl;

	// Gamepads via the Linux joystick interface.
	for (int32_t i = 0; i < 4; ++i)
	{
		int fd = open(wstombs(str(L"/dev/input/js%d", i)).c_str(), O_RDONLY | O_NONBLOCK);
		if (fd < 0)
			break;
		m_devices.push_back(new GamepadDeviceLinux(fd));
		log::info << L"Gamepad " << i << L" input device created." << Endl;
	}

	return true;
}

void InputDriverLinux::setupWayland(const SystemWindow& syswin)
{
	m_display = (wl_display*)syswin.wl_display;
	m_surface = (wl_surface*)syswin.wl_surface;

	m_xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (!m_xkbContext)
		log::warning << L"Input driver (Linux); failed to create xkb context, keyboard text unavailable." << Endl;

	// Route the registry (and everything bound through it) to a private event
	// queue so input dispatch never disturbs the renderer's default-queue /
	// libdecor event handling.
	m_eventQueue = wl_display_create_queue(m_display);

	wl_display* wrapped = (wl_display*)wl_proxy_create_wrapper(m_display);
	wl_proxy_set_queue((wl_proxy*)wrapped, m_eventQueue);
	m_registry = wl_display_get_registry(wrapped);
	wl_proxy_wrapper_destroy(wrapped);

	wl_registry_add_listener(m_registry, &c_registryListener, this);

	// First roundtrip binds globals; second processes seat capabilities and the
	// keyboard keymap.
	wl_display_roundtrip_queue(m_display, m_eventQueue);
	wl_display_roundtrip_queue(m_display, m_eventQueue);

	// Associate a relative pointer with the seat pointer for exclusive (camera) mode.
	if (m_relativePointerManager && m_pointer)
	{
		m_relativePointer = zwp_relative_pointer_manager_v1_get_relative_pointer(m_relativePointerManager, m_pointer);
		if (m_relativePointer)
			zwp_relative_pointer_v1_add_listener(m_relativePointer, &c_relativePointerListener, this);
	}

	if (!m_pointerConstraints || !m_relativePointerManager)
		log::warning << L"Input driver (Linux); compositor lacks pointer-constraints/relative-pointer, exclusive mouse will be limited." << Endl;
}

int InputDriverLinux::getDeviceCount()
{
	return (int)m_devices.size();
}

Ref< IInputDevice > InputDriverLinux::getDevice(int index)
{
	if (index < 0 || index >= (int)m_devices.size())
		return nullptr;
	return m_devices[index];
}

InputDriverLinux::UpdateResult InputDriverLinux::update()
{
	pumpWayland();
	updatePointerLock();
	return UrOk;
}

void InputDriverLinux::setSize(int32_t width, int32_t height)
{
	if (m_mouse)
		m_mouse->setSize(width, height);
}

void InputDriverLinux::pumpWayland()
{
	if (!m_display || !m_eventQueue)
		return;

	wl_display_flush(m_display);

	// Drain events already read into our queue (e.g. by the renderer's per-frame dispatch).
	wl_display_dispatch_queue_pending(m_display, m_eventQueue);

	// Non-blocking read for fresh events. prepare_read_queue/read_events is
	// multi-reader safe and runs sequentially with the renderer's dispatch on
	// the same thread, so it does not conflict with libdecor.
	while (wl_display_prepare_read_queue(m_display, m_eventQueue) != 0)
		wl_display_dispatch_queue_pending(m_display, m_eventQueue);

	wl_display_flush(m_display);

	pollfd pfd;
	pfd.fd = wl_display_get_fd(m_display);
	pfd.events = POLLIN;
	pfd.revents = 0;

	if (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN) != 0)
		wl_display_read_events(m_display);
	else
		wl_display_cancel_read(m_display);

	wl_display_dispatch_queue_pending(m_display, m_eventQueue);
}

void InputDriverLinux::updatePointerLock()
{
	const bool wantLock =
		m_pointer != nullptr &&
		m_surface != nullptr &&
		m_pointerConstraints != nullptr &&
		m_mouse != nullptr &&
		m_mouse->isExclusive() &&
		m_mouse->haveFocus();

	if (wantLock && !m_lockedPointer)
	{
		m_lockedPointer = zwp_pointer_constraints_v1_lock_pointer(
			m_pointerConstraints,
			m_surface,
			m_pointer,
			nullptr,
			ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT
		);
		if (m_lockedPointer)
			zwp_locked_pointer_v1_add_listener(m_lockedPointer, &c_lockedPointerListener, this);
	}
	else if (!wantLock && m_lockedPointer)
	{
		zwp_locked_pointer_v1_destroy(m_lockedPointer);
		m_lockedPointer = nullptr;
		m_pointerLocked = false;
	}
}

void InputDriverLinux::registryGlobal(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);
	if (std::strcmp(interface, wl_seat_interface.name) == 0)
	{
		const uint32_t bind = version < 5 ? version : 5;
		self->m_seat = static_cast< wl_seat* >(wl_registry_bind(registry, name, &wl_seat_interface, bind));
		wl_seat_add_listener(self->m_seat, &c_seatListener, self);
	}
	else if (std::strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0)
	{
		self->m_relativePointerManager = static_cast< zwp_relative_pointer_manager_v1* >(
			wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface, 1));
	}
	else if (std::strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0)
	{
		self->m_pointerConstraints = static_cast< zwp_pointer_constraints_v1* >(
			wl_registry_bind(registry, name, &zwp_pointer_constraints_v1_interface, 1));
	}
}

void InputDriverLinux::registryGlobalRemove(void* /*data*/, wl_registry* /*registry*/, uint32_t /*name*/)
{
}

void InputDriverLinux::seatCapabilities(void* data, wl_seat* seat, uint32_t caps)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);

	if ((caps & WL_SEAT_CAPABILITY_POINTER) != 0 && self->m_pointer == nullptr)
	{
		self->m_pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(self->m_pointer, &c_pointerListener, self);
	}
	else if ((caps & WL_SEAT_CAPABILITY_POINTER) == 0 && self->m_pointer != nullptr)
	{
		wl_pointer_destroy(self->m_pointer);
		self->m_pointer = nullptr;
	}

	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) != 0 && self->m_keyboard == nullptr)
	{
		self->m_keyboard = wl_seat_get_keyboard(seat);
		wl_keyboard_add_listener(self->m_keyboard, &c_keyboardListener, self);
	}
	else if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) == 0 && self->m_keyboard != nullptr)
	{
		wl_keyboard_destroy(self->m_keyboard);
		self->m_keyboard = nullptr;
	}
}

void InputDriverLinux::seatName(void* /*data*/, wl_seat* /*seat*/, const char* /*name*/)
{
}

void InputDriverLinux::pointerEnter(void* data, wl_pointer* /*pointer*/, uint32_t /*serial*/, wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);
	if (!self->m_mouse || surface != self->m_surface)
		return;
	self->m_mouse->setFocus(true);
	self->m_mouse->onPointerEnter(wl_fixed_to_double(sx), wl_fixed_to_double(sy));
}

void InputDriverLinux::pointerLeave(void* data, wl_pointer* /*pointer*/, uint32_t /*serial*/, wl_surface* surface)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);
	if (!self->m_mouse || surface != self->m_surface)
		return;
	self->m_mouse->setFocus(false);
}

void InputDriverLinux::pointerMotion(void* data, wl_pointer* /*pointer*/, uint32_t /*time*/, wl_fixed_t sx, wl_fixed_t sy)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);
	if (self->m_mouse)
		self->m_mouse->onPointerMotion(wl_fixed_to_double(sx), wl_fixed_to_double(sy));
}

void InputDriverLinux::pointerButton(void* data, wl_pointer* /*pointer*/, uint32_t /*serial*/, uint32_t /*time*/, uint32_t button, uint32_t state)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);
	if (self->m_mouse)
		self->m_mouse->onButton(button, state == WL_POINTER_BUTTON_STATE_PRESSED);
}

void InputDriverLinux::pointerAxis(void* data, wl_pointer* /*pointer*/, uint32_t /*time*/, uint32_t axis, wl_fixed_t value)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);
	if (self->m_mouse && axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
		self->m_mouse->onWheel(wl_fixed_to_double(value));
}

void InputDriverLinux::pointerFrame(void* /*data*/, wl_pointer* /*pointer*/)
{
}

void InputDriverLinux::pointerAxisSource(void* /*data*/, wl_pointer* /*pointer*/, uint32_t /*source*/)
{
}

void InputDriverLinux::pointerAxisStop(void* /*data*/, wl_pointer* /*pointer*/, uint32_t /*time*/, uint32_t /*axis*/)
{
}

void InputDriverLinux::pointerAxisDiscrete(void* /*data*/, wl_pointer* /*pointer*/, uint32_t /*axis*/, int32_t /*discrete*/)
{
}

void InputDriverLinux::keyboardKeymap(void* data, wl_keyboard* /*keyboard*/, uint32_t format, int32_t fd, uint32_t size)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);

	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 || !self->m_xkbContext)
	{
		close(fd);
		return;
	}

	char* mapStr = static_cast< char* >(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
	if (mapStr == MAP_FAILED)
	{
		close(fd);
		return;
	}

	if (self->m_xkbState)
	{
		xkb_state_unref(self->m_xkbState);
		self->m_xkbState = nullptr;
	}
	if (self->m_xkbKeymap)
	{
		xkb_keymap_unref(self->m_xkbKeymap);
		self->m_xkbKeymap = nullptr;
	}

	self->m_xkbKeymap = xkb_keymap_new_from_string(self->m_xkbContext, mapStr, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(mapStr, size);
	close(fd);

	if (self->m_xkbKeymap)
		self->m_xkbState = xkb_state_new(self->m_xkbKeymap);
}

void InputDriverLinux::keyboardEnter(void* data, wl_keyboard* /*keyboard*/, uint32_t /*serial*/, wl_surface* surface, wl_array* /*keys*/)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);
	if (self->m_keyboardDevice && (surface == self->m_surface || self->m_surface == nullptr))
		self->m_keyboardDevice->setFocus(true);
}

void InputDriverLinux::keyboardLeave(void* data, wl_keyboard* /*keyboard*/, uint32_t /*serial*/, wl_surface* surface)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);
	if (self->m_keyboardDevice && (surface == self->m_surface || self->m_surface == nullptr))
		self->m_keyboardDevice->setFocus(false);
}

void InputDriverLinux::keyboardKey(void* data, wl_keyboard* /*keyboard*/, uint32_t /*serial*/, uint32_t /*time*/, uint32_t key, uint32_t state)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);
	if (!self->m_keyboardDevice || !self->m_xkbState || !self->m_xkbKeymap)
		return;

	const xkb_keycode_t keycode = key + 8;
	const bool down = (state == WL_KEYBOARD_KEY_STATE_PRESSED);

	// Match against the layout-independent base keysym so e.g. Shift+W still
	// maps to the W control.
	const xkb_layout_index_t layout = xkb_state_key_get_layout(self->m_xkbState, keycode);
	const xkb_keysym_t* syms = nullptr;
	const int32_t n = xkb_keymap_key_get_syms_by_level(self->m_xkbKeymap, keycode, layout, 0, &syms);
	const xkb_keysym_t baseSym = (n > 0) ? syms[0] : XKB_KEY_NoSymbol;

	const int32_t control = controlFromKeysym(baseSym);
	self->m_keyboardDevice->onKey(control, down, false);

	// Emit character (text) events on press, applying the active modifiers/layout.
	if (down)
	{
		char buf[16] = { 0 };
		const int nb = xkb_state_key_get_utf8(self->m_xkbState, keycode, buf, sizeof(buf));
		int i = 0;
		while (i < nb)
		{
			wchar_t wch = 0;
			const int c = Utf8Encoding().translate((const uint8_t*)buf + i, nb - i, wch);
			if (c <= 0)
				break;
			if (wch >= 32 || wch == L'\t' || wch == L'\r' || wch == L'\n' || wch == 8)
				self->m_keyboardDevice->onCharacter(wch);
			i += c;
		}
	}
}

void InputDriverLinux::keyboardModifiers(void* data, wl_keyboard* /*keyboard*/, uint32_t /*serial*/, uint32_t modsDepressed, uint32_t modsLatched, uint32_t modsLocked, uint32_t group)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);
	if (self->m_xkbState)
		xkb_state_update_mask(self->m_xkbState, modsDepressed, modsLatched, modsLocked, 0, 0, group);
}

void InputDriverLinux::keyboardRepeatInfo(void* /*data*/, wl_keyboard* /*keyboard*/, int32_t /*rate*/, int32_t /*delay*/)
{
}

void InputDriverLinux::relativePointerMotion(void* data, zwp_relative_pointer_v1* /*rp*/, uint32_t /*utimeHi*/, uint32_t /*utimeLo*/, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t /*dxUnaccel*/, wl_fixed_t /*dyUnaccel*/)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);
	if (self->m_mouse)
		self->m_mouse->onRelativeMotion(wl_fixed_to_double(dx), wl_fixed_to_double(dy));
}

void InputDriverLinux::lockedPointerLocked(void* data, zwp_locked_pointer_v1* /*lp*/)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);
	self->m_pointerLocked = true;
}

void InputDriverLinux::lockedPointerUnlocked(void* data, zwp_locked_pointer_v1* /*lp*/)
{
	InputDriverLinux* self = static_cast< InputDriverLinux* >(data);
	self->m_pointerLocked = false;
}

}
