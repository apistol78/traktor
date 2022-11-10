/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/UniformBufferChain.h"

namespace traktor::render
{

Ref< UniformBufferChain > UniformBufferChain::create(Context* context, uint32_t blockCount, uint32_t blockSize)
{
	Ref< ApiBuffer > buffer = new ApiBuffer(context);
	if (!buffer->create(
		blockCount * blockSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		true,
		true
	))
	{
		buffer->destroy();
		return nullptr;
	}

	void* top = buffer->lock();
	if (!top)
	{
		buffer->destroy();
		return nullptr;
	}

	return new UniformBufferChain(buffer, top, blockCount, blockSize);
}

void UniformBufferChain::destroy()
{
	if (m_buffer)
	{
		m_buffer->unlock();
		m_buffer->destroy();
		m_buffer = nullptr;
	}
}

bool UniformBufferChain::allocate(UniformBufferRange& outRange)
{
	uint8_t* ptr = (uint8_t*)m_allocator.alloc();
	if (!ptr)
		return false;

	outRange.chain = this;
	outRange.offset = (uint32_t)(ptr - (uint8_t*)m_allocator.top());
	outRange.ptr = ptr;
	return true;
}

void UniformBufferChain::free(const UniformBufferRange& range)
{
	T_ASSERT(range.chain == this);
	m_allocator.free(range.ptr);
}

UniformBufferChain::UniformBufferChain(ApiBuffer* buffer, void* top, uint32_t blockCount, uint32_t blockSize)
:	m_buffer(buffer)
,	m_allocator(top, blockCount, blockSize)
{
}

}
