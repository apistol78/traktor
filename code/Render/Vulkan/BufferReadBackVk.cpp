/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/BufferReadBackVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferReadBackVk", BufferReadBackVk, BufferVk)

BufferReadBackVk::BufferReadBackVk(Context* context, uint32_t bufferSize, uint32_t& instances)
:	BufferVk(context, bufferSize, instances)
{
}

BufferReadBackVk::~BufferReadBackVk()
{
	teardown();
}

bool BufferReadBackVk::create(uint32_t usageBits)
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return false;

	m_buffer = new ApiBuffer(m_context);
	if (!m_buffer->create(bufferSize, usageBits | VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, true))
		return false;

	m_bufferView = BufferViewVk(*m_buffer, 0, bufferSize, bufferSize);

	if ((m_readBack = m_buffer->lock()) == nullptr)
		return false;

	return true;
}

void BufferReadBackVk::destroy()
{
	// Only relinquish ownership; the buffer view handed out to render blocks has
	// to stay valid until every context which might reference it has been
	// rendered. Teardown is performed by the destructor which runs once the
	// retirement fence has been passed. \sa ResourceMorgue
}

void BufferReadBackVk::teardown()
{
	if (m_readBack)
	{
		m_buffer->unlock();
		m_readBack = nullptr;
	}

	safeDestroy(m_buffer);
	m_context = nullptr;
}

void* BufferReadBackVk::lock()
{
	return m_readBack;
}

void BufferReadBackVk::unlock()
{
}

const IBufferView* BufferReadBackVk::getBufferView() const
{
	return &m_bufferView;
}

}
