/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/System/OS.h"
#include "Render/Vulkan/Linux/Window.h"

namespace traktor::render
{
	namespace
	{

const wl_registry_listener c_registryListener =
{
	Window::registryGlobal,
	Window::registryGlobalRemove
};

const wl_seat_listener c_seatListener =
{
	Window::seatCapabilities,
	Window::seatName
};

const wl_pointer_listener c_pointerListener =
{
	.enter = Window::pointerEnter,
	.leave = Window::pointerLeave,
	.motion = Window::pointerMotion,
	.button = Window::pointerButton,
	.axis = Window::pointerAxis
};

libdecor_frame_interface c_frameInterface =
{
	Window::frameConfigure,
	Window::frameClose,
	Window::frameCommit,
	Window::frameDismissPopup,
	nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr
};

libdecor_interface c_libdecorInterface =
{
	Window::libdecorError
};

	}

Window::Window()
:	m_display(nullptr)
,	m_registry(nullptr)
,	m_compositor(nullptr)
,	m_shm(nullptr)
,	m_seat(nullptr)
,	m_pointer(nullptr)
,	m_surface(nullptr)
,	m_cursorTheme(nullptr)
,	m_cursor(nullptr)
,	m_cursorSurface(nullptr)
,	m_libdecor(nullptr)
,	m_frame(nullptr)
,	m_width(0)
,	m_height(0)
,	m_configured(false)
,	m_fullScreen(false)
,	m_active(true)
,	m_cursorShow(true)
,	m_pendingClose(false)
,	m_pendingResize(false)
,	m_enterSerial(0)
{
}

Window::~Window()
{
	if (m_frame)
		libdecor_frame_unref(m_frame);
	if (m_libdecor)
		libdecor_unref(m_libdecor);
	if (m_cursorSurface)
		wl_surface_destroy(m_cursorSurface);
	if (m_cursorTheme)
		wl_cursor_theme_destroy(m_cursorTheme);
	if (m_surface)
		wl_surface_destroy(m_surface);
	if (m_pointer)
		wl_pointer_destroy(m_pointer);
	if (m_seat)
		wl_seat_destroy(m_seat);
	if (m_shm)
		wl_shm_destroy(m_shm);
	if (m_compositor)
		wl_compositor_destroy(m_compositor);
	if (m_registry)
		wl_registry_destroy(m_registry);
	if (m_display)
		wl_display_disconnect(m_display);
}

bool Window::create(uint32_t display, int32_t width, int32_t height)
{
	// On Wayland the client cannot select a specific output for a toplevel;
	// the compositor decides placement. The display index is ignored.
	(void)display;

	// Derive an application identifier from the executable basename. Wayland
	// has no per-window icon protocol; the compositor maps the app id onto an
	// installed .desktop file to resolve the window icon.
	m_appId = wstombs(OS::getInstance().getExecutable().getFileName());

	if ((m_display = wl_display_connect(nullptr)) == nullptr)
	{
		log::error << L"Unable to create Vulkan renderer; failed to connect to Wayland display." << Endl;
		return false;
	}

	m_width = width;
	m_height = height;

	// Bind globals.
	m_registry = wl_display_get_registry(m_display);
	wl_registry_add_listener(m_registry, &c_registryListener, this);

	// First roundtrip to receive globals, second to process seat capabilities etc.
	wl_display_roundtrip(m_display);
	wl_display_roundtrip(m_display);

	if (!m_compositor)
	{
		log::error << L"Unable to create Vulkan renderer; Wayland compositor missing wl_compositor interface." << Endl;
		return false;
	}

	// Create the renderable surface; this becomes the Vulkan presentation surface.
	m_surface = wl_compositor_create_surface(m_compositor);
	if (!m_surface)
	{
		log::error << L"Unable to create Vulkan renderer; failed to create Wayland surface." << Endl;
		return false;
	}

	// Load a cursor theme used when showing the pointer.
	if (m_shm)
	{
		m_cursorTheme = wl_cursor_theme_load(nullptr, 24, m_shm);
		if (m_cursorTheme)
		{
			m_cursor = wl_cursor_theme_get_cursor(m_cursorTheme, "left_ptr");
			m_cursorSurface = wl_compositor_create_surface(m_compositor);
		}
	}

	// Create libdecor context and a decorated toplevel frame. libdecor provides
	// client-side decorations and falls back to server-side decorations when the
	// compositor supports them, so this works across compositors.
	m_libdecor = libdecor_new(m_display, &c_libdecorInterface);
	if (!m_libdecor)
	{
		log::error << L"Unable to create Vulkan renderer; failed to create libdecor context." << Endl;
		return false;
	}

	m_frame = libdecor_decorate(m_libdecor, m_surface, &c_frameInterface, this);
	if (!m_frame)
	{
		log::error << L"Unable to create Vulkan renderer; failed to decorate Wayland toplevel." << Endl;
		return false;
	}

	libdecor_frame_set_title(m_frame, "Traktor - Vulkan Renderer");
	libdecor_frame_set_app_id(m_frame, m_appId.c_str());
	libdecor_frame_map(m_frame);

	// Pump events until the compositor has configured the toplevel, so that the
	// surface has a valid size before the Vulkan swapchain is created.
	int32_t guard = 0;
	while (!m_configured && guard++ < 200)
		libdecor_dispatch(m_libdecor, 100);

	return true;
}

void Window::setTitle(const wchar_t* title)
{
	if (m_frame)
		libdecor_frame_set_title(m_frame, wstombs(title).c_str());
}

void Window::setFullScreenStyle(int32_t /*width*/, int32_t /*height*/)
{
	// The compositor chooses the output and resolution; pass nullptr to let it
	// fullscreen on the output the surface currently occupies.
	if (m_frame)
		libdecor_frame_set_fullscreen(m_frame, nullptr);
	m_fullScreen = true;
}

void Window::setWindowedStyle(int32_t width, int32_t height)
{
	if (m_fullScreen && m_frame)
		libdecor_frame_unset_fullscreen(m_frame);
	m_fullScreen = false;

	// Wayland clients cannot force their own toplevel size; the compositor
	// drives sizing through configure events. Remember the requested size so
	// the next configure can adopt it when the compositor leaves the choice
	// to the client.
	m_width = width;
	m_height = height;
}

void Window::showCursor()
{
	m_cursorShow = true;
	applyCursor();
}

void Window::hideCursor()
{
	m_cursorShow = false;
	applyCursor();
}

void Window::show()
{
	// The surface is mapped by libdecor_frame_map() in create(); it becomes
	// visible once the first frame is presented through the Vulkan swapchain.
	if (m_display)
		wl_display_flush(m_display);
}

void Window::center()
{
	// No-op; Wayland does not allow clients to position their toplevels.
}

bool Window::update(RenderEvent& outEvent)
{
	if (m_display)
		wl_display_flush(m_display);

	// Non-blocking dispatch of both Wayland and libdecor events; this drives
	// the frame configure/close callbacks and our pointer listener.
	if (m_libdecor)
		libdecor_dispatch(m_libdecor, 0);

	if (m_pendingClose)
	{
		m_pendingClose = false;
		outEvent.type = RenderEventType::Close;
		return true;
	}

	if (m_pendingResize)
	{
		m_pendingResize = false;
		outEvent.type = RenderEventType::Resize;
		outEvent.resize.width = m_width;
		outEvent.resize.height = m_height;
		return true;
	}

	return false;
}

void Window::applyCursor()
{
	// Only meaningful while the pointer is over our surface (we have a serial).
	if (!m_pointer || m_enterSerial == 0)
		return;

	if (!m_cursorShow)
	{
		// Hide the pointer by attaching a null cursor surface.
		wl_pointer_set_cursor(m_pointer, m_enterSerial, nullptr, 0, 0);
		return;
	}

	if (m_cursorTheme && m_cursor && m_cursorSurface)
	{
		wl_cursor_image* image = m_cursor->images[0];
		wl_buffer* buffer = wl_cursor_image_get_buffer(image);
		wl_pointer_set_cursor(m_pointer, m_enterSerial, m_cursorSurface, image->hotspot_x, image->hotspot_y);
		wl_surface_attach(m_cursorSurface, buffer, 0, 0);
		wl_surface_damage(m_cursorSurface, 0, 0, image->width, image->height);
		wl_surface_commit(m_cursorSurface);
	}
}

void Window::registryGlobal(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
	Window* self = static_cast< Window* >(data);
	if (std::strcmp(interface, wl_compositor_interface.name) == 0)
		self->m_compositor = static_cast< wl_compositor* >(wl_registry_bind(registry, name, &wl_compositor_interface, 4));
	else if (std::strcmp(interface, wl_shm_interface.name) == 0)
		self->m_shm = static_cast< wl_shm* >(wl_registry_bind(registry, name, &wl_shm_interface, 1));
	else if (std::strcmp(interface, wl_seat_interface.name) == 0)
	{
		// Bind at version 1 so the pointer only emits the core events we handle.
		self->m_seat = static_cast< wl_seat* >(wl_registry_bind(registry, name, &wl_seat_interface, 1));
		wl_seat_add_listener(self->m_seat, &c_seatListener, self);
	}
}

void Window::registryGlobalRemove(void* /*data*/, wl_registry* /*registry*/, uint32_t /*name*/)
{
}

void Window::seatCapabilities(void* data, wl_seat* seat, uint32_t caps)
{
	Window* self = static_cast< Window* >(data);
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
}

void Window::seatName(void* /*data*/, wl_seat* /*seat*/, const char* /*name*/)
{
}

void Window::pointerEnter(void* data, wl_pointer* /*pointer*/, uint32_t serial, wl_surface* surface, wl_fixed_t /*sx*/, wl_fixed_t /*sy*/)
{
	Window* self = static_cast< Window* >(data);
	if (surface != self->m_surface)
		return;
	self->m_enterSerial = serial;
	self->applyCursor();
}

void Window::pointerLeave(void* data, wl_pointer* /*pointer*/, uint32_t /*serial*/, wl_surface* surface)
{
	Window* self = static_cast< Window* >(data);
	if (surface == self->m_surface)
		self->m_enterSerial = 0;
}

void Window::pointerMotion(void* /*data*/, wl_pointer* /*pointer*/, uint32_t /*time*/, wl_fixed_t /*sx*/, wl_fixed_t /*sy*/)
{
}

void Window::pointerButton(void* /*data*/, wl_pointer* /*pointer*/, uint32_t /*serial*/, uint32_t /*time*/, uint32_t /*button*/, uint32_t /*state*/)
{
}

void Window::pointerAxis(void* /*data*/, wl_pointer* /*pointer*/, uint32_t /*time*/, uint32_t /*axis*/, wl_fixed_t /*value*/)
{
}

void Window::frameConfigure(libdecor_frame* frame, libdecor_configuration* configuration, void* userData)
{
	Window* self = static_cast< Window* >(userData);

	int32_t width = 0;
	int32_t height = 0;
	if (!libdecor_configuration_get_content_size(configuration, frame, &width, &height) || width <= 0 || height <= 0)
	{
		// No size proposed yet; keep the requested floating size.
		width = self->m_width;
		height = self->m_height;
	}

	// Track activation and fullscreen state reported by the compositor.
	libdecor_window_state windowState = LIBDECOR_WINDOW_STATE_NONE;
	if (libdecor_configuration_get_window_state(configuration, &windowState))
	{
		self->m_active = (windowState & LIBDECOR_WINDOW_STATE_ACTIVE) != 0;
		self->m_fullScreen = (windowState & LIBDECOR_WINDOW_STATE_FULLSCREEN) != 0;
	}

	if (width != self->m_width || height != self->m_height)
	{
		self->m_width = width;
		self->m_height = height;
		self->m_pendingResize = true;
	}

	libdecor_state* state = libdecor_state_new(width, height);
	libdecor_frame_commit(frame, state, configuration);
	libdecor_state_free(state);

	self->m_configured = true;
}

void Window::frameClose(libdecor_frame* /*frame*/, void* userData)
{
	Window* self = static_cast< Window* >(userData);
	self->m_pendingClose = true;
}

void Window::frameCommit(libdecor_frame* /*frame*/, void* /*userData*/)
{
	// The Vulkan WSI owns commits of the content surface (each vkQueuePresentKHR
	// attaches a buffer and commits), so nothing is committed here.
}

void Window::frameDismissPopup(libdecor_frame* /*frame*/, const char* /*seatName*/, void* /*userData*/)
{
}

void Window::libdecorError(libdecor* /*context*/, libdecor_error /*error*/, const char* message)
{
	log::error << L"libdecor error: " << mbstows(message) << Endl;
}

}
