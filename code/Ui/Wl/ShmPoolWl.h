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
#include "Core/Config.h"

namespace traktor::ui
{

/*! Per-widget pooled SHM buffers for cairo drawing.
 *
 *  Maintains a small set of independently-backed slots (memfd + mmap +
 *  wl_shm_pool + wl_buffer per slot). acquire() rotates to a slot the
 *  compositor is not currently using so a redraw never overwrites pixels
 *  the compositor is still scanning out. Each slot grows on demand
 *  (1.5x amortized, page-aligned) and never shrinks for the pool's
 *  lifetime, so steady-state redraws avoid syscalls entirely.
 *
 *  Usage per frame:
 *    void* data = nullptr;
 *    wl_buffer* buf = pool.acquire(w, h, stride, &data);
 *    // ... draw into 'data' ...
 *    wl_surface_attach(s, buf, 0, 0);
 *    wl_surface_commit(s);
 *    pool.markCommitted();   // active slot is now busy until release.
 */
class ShmPoolWl
{
public:
	explicit ShmPoolWl(wl_shm* shm);

	~ShmPoolWl();

	ShmPoolWl(const ShmPoolWl&) = delete;
	ShmPoolWl& operator=(const ShmPoolWl&) = delete;

	/*! Acquire a free buffer of the requested geometry.
	 *
	 *  Returns a slot the compositor is not holding. The slot's data
	 *  pointer may differ from the previous acquire (or from a previous
	 *  call at the same geometry, if the pool rotated); callers must
	 *  rebind any cairo surface bound to the data pointer when it changes.
	 *
	 *  \return Non-owning wl_buffer*, or nullptr on failure.
	 */
	wl_buffer* acquire(int32_t width, int32_t height, int32_t stride, void** outData);

	/*! Returns the buffer set by the most recent acquire(), or nullptr. */
	wl_buffer* getBuffer() const;

	/*! Mark the active slot as in-use by the compositor.
	 *  Must be called after wl_surface_commit so the slot is not handed
	 *  out by the next acquire() until the compositor releases it. */
	void markCommitted();

private:
	static constexpr int32_t c_slotCount = 2;

	struct Slot
	{
		int fd = -1;
		void* data = nullptr;
		int32_t capacity = 0;
		wl_shm_pool* pool = nullptr;
		wl_buffer* buffer = nullptr;
		int32_t width = 0;
		int32_t height = 0;
		int32_t stride = 0;
		bool busy = false;
	};

	static void onRelease(void* data, wl_buffer* buffer);
	static const wl_buffer_listener s_listener;

	wl_shm* m_shm = nullptr;
	Slot m_slots[c_slotCount];
	Slot* m_active = nullptr;

	bool growSlot(Slot& slot, int32_t newCapacity);
	void destroySlot(Slot& slot);
};

}
