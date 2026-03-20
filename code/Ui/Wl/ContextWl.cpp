/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include "Core/Assert.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/KeyDownEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/KeyUpEvent.h"
#include "Ui/Events/MouseButtonDownEvent.h"
#include "Ui/Events/MouseButtonUpEvent.h"
#include "Ui/Events/MouseMoveEvent.h"
#include "Ui/Wl/ContextWl.h"
#include "Ui/Wl/UtilitiesWl.h"
#include "xdg-shell-client-protocol.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ContextWl", ContextWl, Object)

// Registry listener
static const struct wl_registry_listener s_registryListener = {
	ContextWl::registryGlobal,
	ContextWl::registryGlobalRemove
};

// Seat listener
static const struct wl_seat_listener s_seatListener = {
	ContextWl::seatCapabilities,
	ContextWl::seatName
};

// Pointer listener
static const struct wl_pointer_listener s_pointerListener = {
	ContextWl::pointerEnter,
	ContextWl::pointerLeave,
	ContextWl::pointerMotion,
	ContextWl::pointerButton,
	ContextWl::pointerAxis,
	ContextWl::pointerFrame,
	ContextWl::pointerAxisSource,
	ContextWl::pointerAxisStop,
	ContextWl::pointerAxisDiscrete
};

// Keyboard listener
static const struct wl_keyboard_listener s_keyboardListener = {
	ContextWl::keyboardKeymap,
	ContextWl::keyboardEnter,
	ContextWl::keyboardLeave,
	ContextWl::keyboardKey,
	ContextWl::keyboardModifiers,
	ContextWl::keyboardRepeatInfo
};

// XDG WM base listener
static const struct xdg_wm_base_listener s_xdgWmBaseListener = {
	ContextWl::xdgWmBasePing
};

// Output listener
static const struct wl_output_listener s_outputListener = {
	ContextWl::outputGeometry,
	ContextWl::outputMode,
	ContextWl::outputDone,
	ContextWl::outputScale
};

// libdecor interface
static void libdecorError(struct libdecor* context, enum libdecor_error error, const char* message)
{
	log::error << L"libdecor error: " << mbstows(message) << Endl;
}

static struct libdecor_interface s_libdecorInterface = {
	libdecorError
};

ContextWl::ContextWl()
{
}

ContextWl::~ContextWl()
{
	if (m_xkbState)
		xkb_state_unref(m_xkbState);
	if (m_xkbKeymap)
		xkb_keymap_unref(m_xkbKeymap);
	if (m_xkbContext)
		xkb_context_unref(m_xkbContext);

	if (m_cursorSurface)
		wl_surface_destroy(m_cursorSurface);
	if (m_cursorTheme)
		wl_cursor_theme_destroy(m_cursorTheme);
	if (m_pointer)
		wl_pointer_destroy(m_pointer);
	if (m_keyboard)
		wl_keyboard_destroy(m_keyboard);

	if (m_libdecor)
		libdecor_unref(m_libdecor);

	if (m_display)
		wl_display_disconnect(m_display);
}

bool ContextWl::initialize()
{
	m_display = wl_display_connect(nullptr);
	if (!m_display)
	{
		log::error << L"Failed to connect to Wayland display." << Endl;
		return false;
	}

	m_xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (!m_xkbContext)
	{
		log::error << L"Failed to create xkb context." << Endl;
		return false;
	}

	m_registry = wl_display_get_registry(m_display);
	wl_registry_add_listener(m_registry, &s_registryListener, this);

	// First roundtrip to get globals.
	wl_display_roundtrip(m_display);
	// Second roundtrip to process globals.
	wl_display_roundtrip(m_display);

	if (!m_compositor || !m_xdgWmBase)
	{
		log::error << L"Wayland compositor missing required interfaces." << Endl;
		return false;
	}

	// Create libdecor context for window decorations.
	m_libdecor = libdecor_new(m_display, &s_libdecorInterface);
	if (!m_libdecor)
	{
		log::error << L"Failed to create libdecor context." << Endl;
		return false;
	}

	// Create cursor theme and surface.
	m_cursorTheme = wl_cursor_theme_load(nullptr, 24, m_shm);
	if (m_cursorTheme)
		m_cursorSurface = wl_compositor_create_surface(m_compositor);

	return true;
}

void ContextWl::bind(WidgetData* widget, int32_t eventType, const std::function< void(WlEvent& e) >& fn)
{
	auto& b = m_bindings[widget->surface];
	b.widget = widget;
	b.fns[eventType] = fn;
}

void ContextWl::bindRenderSurface(WidgetData* widget)
{
	auto& b = m_bindings[widget->renderSurface];
	b.widget = widget;
	b.fns = m_bindings[widget->surface].fns;
}

void ContextWl::unbind(WidgetData* widget)
{
	auto it = std::find(m_modal.begin(), m_modal.end(), widget);
	if (it != m_modal.end())
		m_modal.erase(it);

	if (widget->renderSurface)
		m_bindings.remove(widget->renderSurface);

	const bool erased = m_bindings.remove(widget->surface);
	T_FATAL_ASSERT(erased);

	if (m_grabbed == widget)
		m_grabbed = nullptr;

	if (m_pointerFocus == widget)
		m_pointerFocus = nullptr;

	if (m_keyboardFocus == widget)
		m_keyboardFocus = nullptr;

	if (m_internalFocus == widget)
		m_internalFocus = nullptr;

	// Remove from pending expose list.
	auto pe = std::find(m_pendingExposes.begin(), m_pendingExposes.end(), widget);
	if (pe != m_pendingExposes.end())
		m_pendingExposes.erase(pe);
}

void ContextWl::pushModal(WidgetData* widget)
{
	m_modal.push_back(widget);
}

void ContextWl::popModal(WidgetData* widget)
{
	T_FATAL_ASSERT(!m_modal.empty());
	T_FATAL_ASSERT(m_modal.back() == widget);
	m_modal.pop_back();
}

void ContextWl::grab(WidgetData* widget)
{
	T_FATAL_ASSERT(!widget->grabbed);

	widget->grabbed = true;
	if (m_grabbed)
	{
		m_grabbed->grabbed = false;
		m_grabbed = nullptr;
	}
	m_grabbed = widget;
}

void ContextWl::ungrab(WidgetData* widget)
{
	T_FATAL_ASSERT(widget->grabbed);
	T_FATAL_ASSERT(widget == m_grabbed);

	widget->grabbed = false;
	m_grabbed = nullptr;
}

void ContextWl::dispatch(WlEvent& e)
{
	if (!e.surface)
		return;

	switch (e.type)
	{
	case WlEvtFocusIn:
	case WlEvtFocusOut:
	case WlEvtConfigure:
	case WlEvtClose:
	case WlEvtExpose:
	case WlEvtPointerEnter:
	case WlEvtPointerLeave:
		dispatch(e.surface, e.type, true, e);
		break;

	default:
		dispatch(e.surface, e.type, false, e);
		break;
	}
}

void ContextWl::enqueueEvent(const WlEvent& e)
{
	m_eventQueue.push_back(e);
}

bool ContextWl::processEventQueue(EventSubject* owner)
{
	if (m_eventQueue.empty())
		return false;

	AlignedVector< WlEvent > queue;
	std::swap(queue, m_eventQueue);

	for (auto& e : queue)
	{
		if (!preTranslateEvent(owner, e))
			dispatch(e);
	}
	return true;
}

bool ContextWl::preTranslateEvent(EventSubject* owner, WlEvent& e)
{
	if (anyGrabbed())
		return false;

	if (e.type == WlEvtPointerMotion)
	{
		int32_t button = 0;
		// Pointer coordinates are surface-local; use them as-is for the global event
		// since Wayland doesn't provide screen-global coordinates.
		MouseMoveEvent mouseMoveEvent(
			owner,
			button,
			Point((int)e.pointerX, (int)e.pointerY)
		);
		owner->raiseEvent(&mouseMoveEvent);
	}
	else if (e.type == WlEvtPointerButton)
	{
		int32_t button = 0;
		switch (e.button)
		{
		case 0x110: button = MbtLeft; break;   // BTN_LEFT
		case 0x112: button = MbtMiddle; break; // BTN_MIDDLE
		case 0x111: button = MbtRight; break;  // BTN_RIGHT
		default: break;
		}

		if (e.buttonState == WL_POINTER_BUTTON_STATE_PRESSED)
		{
			MouseButtonDownEvent mouseButtonDownEvent(
				owner,
				button,
				Point((int)e.pointerX, (int)e.pointerY)
			);
			owner->raiseEvent(&mouseButtonDownEvent);
		}
		else
		{
			MouseButtonUpEvent mouseButtonUpEvent(
				owner,
				button,
				Point((int)e.pointerX, (int)e.pointerY)
			);
			owner->raiseEvent(&mouseButtonUpEvent);
		}
	}
	else if (e.type == WlEvtKeyboardKey)
	{
		if (!m_xkbState)
			return false;

		const uint32_t keycode = e.key + 8;
		const xkb_keysym_t sym = xkb_state_key_get_one_sym(m_xkbState, keycode);
		VirtualKey vk = translateToVirtualKey(sym);

		bool consumed = false;

		if (e.keyState == WL_KEYBOARD_KEY_STATE_PRESSED)
		{
			if (vk != VkNull)
			{
				KeyDownEvent keyDownEvent(owner, vk, e.key, 0);
				owner->raiseEvent(&keyDownEvent);
				consumed |= keyDownEvent.consumed();
			}

			char buf[8] = { 0 };
			int n = xkb_state_key_get_utf8(m_xkbState, keycode, buf, sizeof(buf));
			if (n > 0)
			{
				wchar_t wch = 0;
				if (Utf8Encoding().translate((const uint8_t*)buf, n, wch) > 0)
				{
					KeyEvent keyEvent(owner, vk, e.key, wch);
					owner->raiseEvent(&keyEvent);
					consumed |= keyEvent.consumed();
				}
			}
		}
		else
		{
			if (vk != VkNull)
			{
				KeyUpEvent keyUpEvent(owner, vk, e.key, 0, false);
				owner->raiseEvent(&keyUpEvent);
				consumed |= keyUpEvent.consumed();
			}
		}

		return consumed;
	}

	return false;
}

struct wl_buffer* ContextWl::createShmBuffer(int32_t width, int32_t height, int32_t stride, void** outData)
{
	const int32_t size = stride * height;

	int fd = memfd_create("traktor-wl-shm", MFD_CLOEXEC);
	if (fd < 0)
		return nullptr;

	if (ftruncate(fd, size) < 0)
	{
		close(fd);
		return nullptr;
	}

	void* data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED)
	{
		close(fd);
		return nullptr;
	}

	struct wl_shm_pool* pool = wl_shm_create_pool(m_shm, fd, size);
	struct wl_buffer* buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);
	close(fd);

	*outData = data;
	return buffer;
}

void ContextWl::queueExpose(WidgetData* widget)
{
	// Avoid duplicates.
	for (auto* w : m_pendingExposes)
	{
		if (w == widget)
			return;
	}
	m_pendingExposes.push_back(widget);
}

bool ContextWl::processPendingExposes()
{
	if (m_pendingExposes.empty())
		return false;

	// Copy and clear to handle reentrant calls from draw.
	AlignedVector< WidgetData* > pending;
	std::swap(pending, m_pendingExposes);

	for (auto* widget : pending)
	{
		WlEvent e;
		e.type = WlEvtExpose;
		e.surface = widget->surface;
		dispatch(e);
	}
	return true;
}

void ContextWl::setInternalFocus(WidgetData* widget)
{
	if (widget == m_internalFocus)
		return;

	// Send FocusOut to the previously focused widget.
	if (m_internalFocus)
	{
		WlEvent e;
		e.type = WlEvtFocusOut;
		e.surface = m_internalFocus->surface;
		dispatch(e);
	}

	m_internalFocus = widget;

	// Send FocusIn to the newly focused widget.
	if (m_internalFocus)
	{
		WlEvent e;
		e.type = WlEvtFocusIn;
		e.surface = m_internalFocus->surface;
		dispatch(e);
	}
}

int32_t ContextWl::getSystemDPI() const
{
	return m_dpi;
}

void ContextWl::dispatch(struct wl_surface* surface, int32_t eventType, bool always, WlEvent& e)
{
	auto b = m_bindings.find(surface);
	if (b == m_bindings.end())
		return;

	const auto& binding = b->second;
	T_FATAL_ASSERT(binding.widget != nullptr);

	if (!always && binding.widget != m_grabbed)
	{
		for (const WidgetData* w = binding.widget; w != nullptr; w = w->parent)
		{
			if (!w->enable)
				return;
		}

		if (!m_modal.empty())
		{
			bool p = false;
			for (const WidgetData* w = binding.widget; w != nullptr; w = w->parent)
			{
				if (w == m_modal.back())
				{
					p = true;
					break;
				}
			}
			if (!p)
				return;
		}
	}

	auto d = binding.fns.find(eventType);
	if (d != binding.fns.end())
	{
		std::function< void(WlEvent& e) > fn = d->second;
		fn(e);
	}
}

// Registry
void ContextWl::registryGlobal(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	if (std::strcmp(interface, wl_compositor_interface.name) == 0)
		ctx->m_compositor = static_cast< struct wl_compositor* >(wl_registry_bind(registry, name, &wl_compositor_interface, 4));
	else if (std::strcmp(interface, wl_subcompositor_interface.name) == 0)
		ctx->m_subcompositor = static_cast< struct wl_subcompositor* >(wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
	else if (std::strcmp(interface, wl_shm_interface.name) == 0)
		ctx->m_shm = static_cast< struct wl_shm* >(wl_registry_bind(registry, name, &wl_shm_interface, 1));
	else if (std::strcmp(interface, xdg_wm_base_interface.name) == 0)
	{
		ctx->m_xdgWmBase = static_cast< struct xdg_wm_base* >(wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));
		xdg_wm_base_add_listener(ctx->m_xdgWmBase, &s_xdgWmBaseListener, ctx);
	}
	else if (std::strcmp(interface, wl_seat_interface.name) == 0)
	{
		ctx->m_seat = static_cast< struct wl_seat* >(wl_registry_bind(registry, name, &wl_seat_interface, 5));
		wl_seat_add_listener(ctx->m_seat, &s_seatListener, ctx);
	}
	else if (std::strcmp(interface, wl_output_interface.name) == 0)
	{
		ctx->m_output = static_cast< struct wl_output* >(wl_registry_bind(registry, name, &wl_output_interface, 2));
		wl_output_add_listener(ctx->m_output, &s_outputListener, ctx);
	}
}

void ContextWl::registryGlobalRemove(void* data, struct wl_registry* registry, uint32_t name)
{
}

// Seat
void ContextWl::seatCapabilities(void* data, struct wl_seat* seat, uint32_t caps)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	if ((caps & WL_SEAT_CAPABILITY_POINTER) && !ctx->m_pointer)
	{
		ctx->m_pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(ctx->m_pointer, &s_pointerListener, ctx);
	}

	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !ctx->m_keyboard)
	{
		ctx->m_keyboard = wl_seat_get_keyboard(seat);
		wl_keyboard_add_listener(ctx->m_keyboard, &s_keyboardListener, ctx);
	}
}

void ContextWl::seatName(void* data, struct wl_seat* seat, const char* name)
{
}

// Pointer
void ContextWl::pointerEnter(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);
	ctx->m_pointerSerial = serial;

	ctx->m_pointerX = wl_fixed_to_double(sx) * ctx->m_outputScale;
	ctx->m_pointerY = wl_fixed_to_double(sy) * ctx->m_outputScale;

	auto b = ctx->m_bindings.find(surface);
	if (b != ctx->m_bindings.end())
		ctx->m_pointerFocus = b->second.widget;

	WlEvent e;
	e.type = WlEvtPointerEnter;
	e.surface = surface;
	e.pointerX = ctx->m_pointerX;
	e.pointerY = ctx->m_pointerY;
	e.serial = serial;
	ctx->enqueueEvent(e);
}

void ContextWl::pointerLeave(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);
	ctx->m_pointerSerial = serial;

	WlEvent e;
	e.type = WlEvtPointerLeave;
	e.surface = surface;
	e.serial = serial;
	ctx->enqueueEvent(e);

	if (ctx->m_pointerFocus)
	{
		auto b = ctx->m_bindings.find(surface);
		if (b != ctx->m_bindings.end() && b->second.widget == ctx->m_pointerFocus)
			ctx->m_pointerFocus = nullptr;
	}
}

void ContextWl::pointerMotion(void* data, struct wl_pointer* pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	struct wl_surface* surface = ctx->m_pointerFocus ? ctx->m_pointerFocus->surface : nullptr;
	if (ctx->m_grabbed)
		surface = ctx->m_grabbed->surface;

	if (!surface)
		return;

	ctx->m_pointerX = wl_fixed_to_double(sx) * ctx->m_outputScale;
	ctx->m_pointerY = wl_fixed_to_double(sy) * ctx->m_outputScale;

	WlEvent e;
	e.type = WlEvtPointerMotion;
	e.surface = surface;
	e.pointerX = ctx->m_pointerX;
	e.pointerY = ctx->m_pointerY;
	ctx->enqueueEvent(e);
}

void ContextWl::pointerButton(void* data, struct wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);
	ctx->m_pointerSerial = serial;

	struct wl_surface* surface = ctx->m_pointerFocus ? ctx->m_pointerFocus->surface : nullptr;
	if (ctx->m_grabbed)
		surface = ctx->m_grabbed->surface;

	if (!surface)
		return;

	WlEvent e;
	e.type = WlEvtPointerButton;
	e.surface = surface;
	e.pointerX = ctx->m_pointerX;
	e.pointerY = ctx->m_pointerY;
	e.button = button;
	e.buttonState = state;
	e.serial = serial;
	ctx->enqueueEvent(e);
}

void ContextWl::pointerAxis(void* data, struct wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	struct wl_surface* surface = ctx->m_pointerFocus ? ctx->m_pointerFocus->surface : nullptr;
	if (!surface)
		return;

	WlEvent e;
	e.type = WlEvtPointerAxis;
	e.surface = surface;
	e.pointerX = ctx->m_pointerX;
	e.pointerY = ctx->m_pointerY;
	e.axisType = axis;
	e.axisValue = wl_fixed_to_double(value);
	ctx->enqueueEvent(e);
}

void ContextWl::pointerFrame(void* data, struct wl_pointer* pointer) {}
void ContextWl::pointerAxisSource(void* data, struct wl_pointer* pointer, uint32_t source) {}
void ContextWl::pointerAxisStop(void* data, struct wl_pointer* pointer, uint32_t time, uint32_t axis) {}
void ContextWl::pointerAxisDiscrete(void* data, struct wl_pointer* pointer, uint32_t axis, int32_t discrete) {}

// Keyboard
void ContextWl::keyboardKeymap(void* data, struct wl_keyboard* keyboard, uint32_t format, int32_t fd, uint32_t size)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
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

	if (ctx->m_xkbState)
	{
		xkb_state_unref(ctx->m_xkbState);
		ctx->m_xkbState = nullptr;
	}
	if (ctx->m_xkbKeymap)
	{
		xkb_keymap_unref(ctx->m_xkbKeymap);
		ctx->m_xkbKeymap = nullptr;
	}

	ctx->m_xkbKeymap = xkb_keymap_new_from_string(ctx->m_xkbContext, mapStr, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(mapStr, size);
	close(fd);

	if (ctx->m_xkbKeymap)
		ctx->m_xkbState = xkb_state_new(ctx->m_xkbKeymap);
}

void ContextWl::keyboardEnter(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	auto b = ctx->m_bindings.find(surface);
	if (b != ctx->m_bindings.end())
	{
		ctx->m_keyboardFocus = b->second.widget;
	}
	else
	{
		// Surface not in bindings — libdecor CSD creates an internal
		// parent surface that we don't track. Fall back to the first
		// toplevel widget so keyboard routing still works.
		for (auto it = ctx->m_bindings.begin(); it != ctx->m_bindings.end(); ++it)
		{
			if (it->second.widget && it->second.widget->topLevel)
			{
				ctx->m_keyboardFocus = it->second.widget;
				break;
			}
		}
	}

	// If no child widget has internal focus, give it to the toplevel.
	if (!ctx->m_internalFocus && ctx->m_keyboardFocus)
		ctx->m_internalFocus = ctx->m_keyboardFocus;

	// Dispatch FocusIn to the internally focused widget.
	if (ctx->m_internalFocus)
	{
		WlEvent e;
		e.type = WlEvtFocusIn;
		e.surface = ctx->m_internalFocus->surface;
		e.serial = serial;
		ctx->enqueueEvent(e);
	}
}

void ContextWl::keyboardLeave(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	// Dispatch FocusOut to the internally focused widget.
	if (ctx->m_internalFocus)
	{
		WlEvent e;
		e.type = WlEvtFocusOut;
		e.surface = ctx->m_internalFocus->surface;
		e.serial = serial;
		ctx->enqueueEvent(e);
	}

	ctx->m_keyboardFocus = nullptr;
	ctx->m_internalFocus = nullptr;
}

void ContextWl::keyboardKey(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	// Route keyboard events to the internally focused widget,
	// falling back to the compositor-level keyboard focus toplevel.
	WidgetData* target = ctx->m_internalFocus ? ctx->m_internalFocus : ctx->m_keyboardFocus;
	if (!target)
		return;

	WlEvent e;
	e.type = WlEvtKeyboardKey;
	e.surface = target->surface;
	e.key = key;
	e.keyState = state;
	e.serial = serial;
	ctx->enqueueEvent(e);
}

void ContextWl::keyboardModifiers(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t modsDepressed, uint32_t modsLatched, uint32_t modsLocked, uint32_t group)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	if (ctx->m_xkbState)
		xkb_state_update_mask(ctx->m_xkbState, modsDepressed, modsLatched, modsLocked, 0, 0, group);

	ctx->m_keyboardModifiers = modsDepressed;
}

void ContextWl::keyboardRepeatInfo(void* data, struct wl_keyboard* keyboard, int32_t rate, int32_t delay) {}

// XDG WM base
void ContextWl::xdgWmBasePing(void* data, struct xdg_wm_base* wmBase, uint32_t serial)
{
	xdg_wm_base_pong(wmBase, serial);
}

// Output
void ContextWl::outputGeometry(void* data, struct wl_output* output, int32_t x, int32_t y, int32_t pw, int32_t ph, int32_t subpixel, const char* make, const char* model, int32_t transform)
{
}

void ContextWl::outputMode(void* data, struct wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
}

void ContextWl::outputDone(void* data, struct wl_output* output)
{
}

void ContextWl::outputScale(void* data, struct wl_output* output, int32_t factor)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);
	ctx->m_outputScale = factor;
	// Set effective DPI so dpi96() returns scale-adjusted values.
	// This is needed by code that rasterizes bitmaps/SVGs at the
	// correct physical resolution (e.g. StyleBitmap).
	ctx->m_dpi = 96 * factor;
}

}
