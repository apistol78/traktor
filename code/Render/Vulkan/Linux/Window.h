/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include <libdecor.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include "Core/Object.h"
#include "Render/Types.h"

namespace traktor::render
{

/*! Standalone Wayland render window.
 * \ingroup Render
 *
 * Native Wayland implementation of the stand-alone Vulkan render window.
 * Owns its own Wayland connection (analogous to the X11 implementation
 * opening its own display) and uses libdecor to provide a decorated
 * xdg-shell toplevel that works across compositors regardless of whether
 * they support server-side decorations.
 *
 * The embedded render-view path (RenderViewEmbeddedDesc) is unaffected;
 * it continues to receive a wl_surface from the UI toolkit.
 */
class Window : public Object
{
public:
	Window();

	virtual ~Window();

	bool create(uint32_t display, int32_t width, int32_t height);

	void setTitle(const wchar_t* title);

	void setFullScreenStyle(int32_t width, int32_t height);

	void setWindowedStyle(int32_t width, int32_t height);

	void showCursor();

	void hideCursor();

	void show();

	void center();

	bool update(RenderEvent& outEvent);

	int32_t getWidth() const { return m_width; }

	int32_t getHeight() const { return m_height; }

	bool isFullScreen() const { return m_fullScreen; }

	bool isActive() const { return m_active; }

	wl_display* getDisplay() const { return m_display; }

	wl_surface* getSurface() const { return m_surface; }

private:
	wl_display* m_display;
	wl_registry* m_registry;
	wl_compositor* m_compositor;
	wl_shm* m_shm;
	wl_seat* m_seat;
	wl_pointer* m_pointer;
	wl_surface* m_surface;
	wl_cursor_theme* m_cursorTheme;
	wl_cursor* m_cursor;
	wl_surface* m_cursorSurface;
	libdecor* m_libdecor;
	libdecor_frame* m_frame;

	std::string m_appId;

	int32_t m_width;
	int32_t m_height;
	bool m_configured;
	bool m_fullScreen;
	bool m_active;
	bool m_cursorShow;

	// Pending events surfaced through update().
	bool m_pendingClose;
	bool m_pendingResize;
	uint32_t m_enterSerial;

	void applyCursor();

public:
	// Wayland C-style listener callbacks (public so the file-scope listener
	// tables can reference them; they take a Window* through the userdata arg).

	// Registry listener.
	static void registryGlobal(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
	static void registryGlobalRemove(void* data, wl_registry* registry, uint32_t name);

	// Seat / pointer listener.
	static void seatCapabilities(void* data, wl_seat* seat, uint32_t caps);
	static void seatName(void* data, wl_seat* seat, const char* name);
	static void pointerEnter(void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy);
	static void pointerLeave(void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface);
	static void pointerMotion(void* data, wl_pointer* pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
	static void pointerButton(void* data, wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
	static void pointerAxis(void* data, wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value);

	// libdecor frame interface.
	static void frameConfigure(libdecor_frame* frame, libdecor_configuration* configuration, void* userData);
	static void frameClose(libdecor_frame* frame, void* userData);
	static void frameCommit(libdecor_frame* frame, void* userData);
	static void frameDismissPopup(libdecor_frame* frame, const char* seatName, void* userData);

	// libdecor interface.
	static void libdecorError(libdecor* context, libdecor_error error, const char* message);
};

}
