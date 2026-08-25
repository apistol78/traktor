/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
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
	m_top = nullptr;
	m_free.clear();
}

bool UniformBufferChain::allocate(UniformBufferRange& outRange)
{
	if (m_free.empty())
		return false;

	const uint32_t index = m_free.back();
	m_free.pop_back();

	outRange.chain = this;
	outRange.offset = index * m_blockSize;
	outRange.ptr = m_top + outRange.offset;
	return true;
}

void UniformBufferChain::free(const UniformBufferRange& range)
{
	T_ASSERT(range.chain == this);
	T_ASSERT((range.offset % m_blockSize) == 0);
	m_free.push_back(range.offset / m_blockSize);
}

UniformBufferChain::UniformBufferChain(ApiBuffer* buffer, void* top, uint32_t blockCount, uint32_t blockSize)
:	m_buffer(buffer)
,	m_top((uint8_t*)top)
,	m_blockSize(blockSize)
{
	m_free.resize(blockCount);
	for (uint32_t i = 0; i < blockCount; ++i)
		m_free[i] = blockCount - 1 - i;
}

}
