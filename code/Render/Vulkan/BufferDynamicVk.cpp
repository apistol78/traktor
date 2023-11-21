/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/BufferDynamicVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Context.h"

namespace traktor::render
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferDynamicVk", BufferDynamicVk, BufferVk)

BufferDynamicVk::BufferDynamicVk(Context* context, uint32_t bufferSize, uint32_t& instances)
:	BufferVk(context, bufferSize, instances)
{
}

BufferDynamicVk::~BufferDynamicVk()
{
	destroy();
}

bool BufferDynamicVk::create(uint32_t usageBits, int32_t inFlightCount)
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return false;

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(m_context->getPhysicalDevice(), &deviceProperties);
	const uint32_t storageBufferOffsetAlignment = (uint32_t)deviceProperties.limits.minStorageBufferOffsetAlignment;

	// Add an extra buffer since threaded rendering might queue an extra frame worth of rendering.
	m_inFlightCount = inFlightCount + 1;
	m_range = alignUp(bufferSize, storageBufferOffsetAlignment);
	
	const uint32_t size = m_range * m_inFlightCount;

	m_buffer = new ApiBuffer(m_context);
	if (!m_buffer->create(size, usageBits, true, true))
		return false;

	m_bufferViews = new BufferViewVk [m_inFlightCount];
	for (uint32_t i = 0; i < m_inFlightCount; ++i)
	{
		m_bufferViews[i] = BufferViewVk(
			*m_buffer,
			i * m_range,
			m_range,
			size
		);
	}

	m_ptr = (uint8_t*)m_buffer->lock();
	if (!m_ptr)
		return false;

	return true;
}

void BufferDynamicVk::destroy()
{
	if (m_bufferViews)
	{
		delete[] m_bufferViews;
		m_bufferViews = nullptr;
	}

	if (m_buffer)
	{
		m_buffer->unlock();
		m_buffer->destroy();
		m_buffer = nullptr;
	}

	m_context = nullptr;
	m_ptr = nullptr;
}

void* BufferDynamicVk::lock()
{
	return m_ptr + m_index * m_range;
}

void BufferDynamicVk::unlock()
{
	m_view = m_index;
	m_index = (m_index + 1) % m_inFlightCount;
}

const IBufferView* BufferDynamicVk::getBufferView() const
{
	return &m_bufferViews[m_view];
}

}
