/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <sys/mman.h>
#include <unistd.h>
#include "Ui/Wl/ShmPoolWl.h"

namespace traktor::ui
{
namespace
{

constexpr int32_t c_pageSize = 4096;

int32_t alignUp(int32_t v, int32_t align)
{
	return (v + (align - 1)) & ~(align - 1);
}

}

const wl_buffer_listener ShmPoolWl::s_listener = {
	&ShmPoolWl::onRelease
};

void ShmPoolWl::onRelease(void* data, wl_buffer* /*buffer*/)
{
	static_cast< Slot* >(data)->busy = false;
}

ShmPoolWl::ShmPoolWl(wl_shm* shm)
:	m_shm(shm)
{
}

ShmPoolWl::~ShmPoolWl()
{
	for (auto& slot : m_slots)
		destroySlot(slot);
}

wl_buffer* ShmPoolWl::getBuffer() const
{
	return m_active != nullptr ? m_active->buffer : nullptr;
}

void ShmPoolWl::markCommitted()
{
	if (m_active != nullptr)
		m_active->busy = true;
}

wl_buffer* ShmPoolWl::acquire(int32_t width, int32_t height, int32_t stride, void** outData)
{
	if (m_shm == nullptr || width <= 0 || height <= 0 || stride < width * 4)
		return nullptr;

	// Fast path: re-use the currently active slot if it isn't held by the
	// compositor and already has matching geometry. Keeps cairo binding stable.
	if (m_active != nullptr && !m_active->busy && m_active->buffer != nullptr &&
		m_active->width == width && m_active->height == height && m_active->stride == stride)
	{
		*outData = m_active->data;
		return m_active->buffer;
	}

	// Otherwise pick any non-busy slot, preferring one that already matches the
	// requested geometry (avoids destroying and re-creating its wl_buffer).
	Slot* chosen = nullptr;
	for (auto& s : m_slots)
	{
		if (!s.busy && s.buffer != nullptr &&
			s.width == width && s.height == height && s.stride == stride)
		{
			chosen = &s;
			break;
		}
	}
	if (chosen == nullptr)
	{
		for (auto& s : m_slots)
		{
			if (!s.busy)
			{
				chosen = &s;
				break;
			}
		}
	}

	// All slots are still held by the compositor; fall back to the active
	// slot. This re-introduces the resize race, but only when the compositor
	// is more than c_slotCount frames behind, which is rare in practice.
	if (chosen == nullptr)
		chosen = m_active != nullptr ? m_active : &m_slots[0];

	const int32_t needed = stride * height;
	if (needed > chosen->capacity)
	{
		const int32_t target = std::max(needed, chosen->capacity + chosen->capacity / 2);
		if (!growSlot(*chosen, alignUp(target, c_pageSize)))
			return nullptr;
	}

	if (chosen->buffer == nullptr ||
		chosen->width != width || chosen->height != height || chosen->stride != stride)
	{
		wl_buffer* newBuffer = wl_shm_pool_create_buffer(chosen->pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
		if (newBuffer == nullptr)
			return nullptr;
		if (chosen->buffer != nullptr)
			wl_buffer_destroy(chosen->buffer);
		chosen->buffer = newBuffer;
		chosen->width = width;
		chosen->height = height;
		chosen->stride = stride;
		wl_buffer_add_listener(chosen->buffer, &s_listener, chosen);
	}

	m_active = chosen;
	*outData = chosen->data;
	return chosen->buffer;
}

bool ShmPoolWl::growSlot(Slot& slot, int32_t newCapacity)
{
	if (newCapacity <= slot.capacity)
		return true;

	if (slot.fd < 0)
	{
		slot.fd = memfd_create("traktor-wl-shm", MFD_CLOEXEC);
		if (slot.fd < 0)
			return false;
		if (ftruncate(slot.fd, newCapacity) < 0)
		{
			close(slot.fd);
			slot.fd = -1;
			return false;
		}
		slot.data = mmap(nullptr, newCapacity, PROT_READ | PROT_WRITE, MAP_SHARED, slot.fd, 0);
		if (slot.data == MAP_FAILED)
		{
			slot.data = nullptr;
			close(slot.fd);
			slot.fd = -1;
			return false;
		}
		slot.pool = wl_shm_create_pool(m_shm, slot.fd, newCapacity);
		if (slot.pool == nullptr)
		{
			munmap(slot.data, newCapacity);
			slot.data = nullptr;
			close(slot.fd);
			slot.fd = -1;
			return false;
		}
		slot.capacity = newCapacity;
		return true;
	}

	if (ftruncate(slot.fd, newCapacity) < 0)
		return false;

	void* newData = mremap(slot.data, slot.capacity, newCapacity, MREMAP_MAYMOVE);
	if (newData == MAP_FAILED)
		return false;

	slot.data = newData;
	slot.capacity = newCapacity;
	wl_shm_pool_resize(slot.pool, newCapacity);
	return true;
}

void ShmPoolWl::destroySlot(Slot& slot)
{
	if (slot.buffer != nullptr)
		wl_buffer_destroy(slot.buffer);
	if (slot.pool != nullptr)
		wl_shm_pool_destroy(slot.pool);
	if (slot.data != nullptr && slot.capacity > 0)
		munmap(slot.data, slot.capacity);
	if (slot.fd >= 0)
		close(slot.fd);
	slot = Slot{};
}

}
