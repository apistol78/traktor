/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <wayland-client.h>
#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor::ui
{

class Context : public Object
{
	T_RTTI_CLASS;

public:
	virtual ~Context();

	static Ref< Context > create();

	wl_buffer* allocateBuffer(int32_t width, int32_t height);

	wl_surface* createSurface(int32_t width, int32_t height);

	//@{

	wl_display* getDisplay() const { return m_display; }

	wl_compositor* getCompositor() const { return m_compositor; }

	wl_shm* getSHM() const { return m_shm; }

	wl_shell* getShell() const { return m_shell; }

	//@}

private:
	wl_display* m_display = nullptr;
	wl_compositor* m_compositor = nullptr;
	wl_shm* m_shm = nullptr;
	wl_shell* m_shell = nullptr;

	wl_shm_pool* m_pool = nullptr;

	static void registryGlobalHandler(
		void* data,
		wl_registry* registry,
		uint32_t name,
		const char* interface,
		uint32_t version
	);

	static void registryGlobalRemoveHandler(
		void* data,
		wl_registry* registry,
		uint32_t name
	);
};

}
