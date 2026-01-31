/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <syscall.h>
#include <unistd.h>
#include <sys/mman.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/WL/Context.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Context", Context, Object)

Context::~Context()
{
	wl_display_disconnect(m_display);
}

Ref< Context > Context::create()
{
	Ref< Context > context = new Context();

   	context->m_display = wl_display_connect(nullptr);
	if (!context->m_display)
		return nullptr;

	wl_registry* registry = wl_display_get_registry(context->m_display);
	wl_registry_listener registry_listener =
	{
		.global = registryGlobalHandler,
		.global_remove = registryGlobalRemoveHandler
	};
	wl_registry_add_listener(registry, &registry_listener, context.ptr());

	wl_display_roundtrip(context->m_display);

	if (
		!context->m_compositor ||
		!context->m_shm ||
		!context->m_xdg_wm_base
	)
		return nullptr;

	xdg_wm_base_listener xdgWMBaseListener = {
		.ping = xdgWMBaseHandlePing,
	};
	xdg_wm_base_add_listener(context->m_xdg_wm_base, &xdgWMBaseListener, context.ptr());

	wl_display_roundtrip(context->m_display);

	const int32_t size = 1 * 1024 * 1024;

	// open an anonymous file and write some zero bytes to it
	int32_t fd = syscall(SYS_memfd_create, "buffer", 0);
	ftruncate(fd, size);

	// map it to the memory
	uint8_t* data = (uint8_t*)mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (!data)
		return nullptr;

	context->m_pool = wl_shm_create_pool(context->m_shm, fd, size);
	if (!context->m_pool)
		return nullptr;

	return context;
}

wl_buffer* Context::allocateBuffer(int32_t width, int32_t height)
{
	return wl_shm_pool_create_buffer(
		m_pool,
    	0,
		width,
		height,
		width * 4,
		WL_SHM_FORMAT_XRGB8888
	);
}

wl_surface* Context::createSurface(int32_t width, int32_t height)
{
	wl_buffer* buffer = allocateBuffer(width, height);
	if (!buffer)
		return nullptr;

	wl_surface* surface = wl_compositor_create_surface(m_compositor);
	if (!surface)
		return nullptr;

	wl_surface_attach(surface, buffer, 0, 0);
	wl_surface_commit(surface);
	return surface;
}

void Context::registryGlobalHandler(
	void* data,
	wl_registry* registry,
	uint32_t name,
	const char* interface,
	uint32_t version
)
{
	Context* context = (Context*)data;

	log::info << L"Interface \"" << mbstows(interface) << L"\", version " << version << L", name \"" << name << L"\"." << Endl;

	if (strcmp(interface, wl_compositor_interface.name) == 0)
	{
		context->m_compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 3);
	}
	else if (strcmp(interface, wl_shm_interface.name) == 0)
	{
		context->m_shm = (wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, 1);
	}
	else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
	{
		context->m_xdg_wm_base = (xdg_wm_base*)wl_registry_bind(registry, name, &wl_shell_interface, 1);
	}
}

void Context::registryGlobalRemoveHandler(
	void* data,
	wl_registry* registry,
	uint32_t name
)
{
	Context* context = (Context*)data;
}

void Context::xdgWMBaseHandlePing(
	void *data,
	xdg_wm_base* xdg_wm_base,
	uint32_t serial
)
{
	xdg_wm_base_pong(xdg_wm_base, serial);
}

}
