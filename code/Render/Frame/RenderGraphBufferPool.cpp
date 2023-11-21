/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Frame/RenderGraphBufferPool.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderGraphBufferPool", RenderGraphBufferPool, Object)

RenderGraphBufferPool::RenderGraphBufferPool(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

void RenderGraphBufferPool::destroy()
{
	m_renderSystem = nullptr;
}

Ref< Buffer > RenderGraphBufferPool::acquire(uint32_t bufferSize, uint32_t persistentHandle)
{
	auto it = std::find_if(
		m_pool.begin(),
		m_pool.end(),
		[&](const RenderGraphBufferPool::Pool& p)
		{
			if (p.persistentHandle != persistentHandle)
				return false;

			if (p.bufferSize != bufferSize)
				return false;

			return true;
		}
	);

	// Get or create pool.
	Pool* pool = nullptr;
	if (it != m_pool.end())
		pool = &(*it);
	else
	{
		pool = &m_pool.push_back();
		pool->bufferSize = bufferSize;
		pool->persistentHandle = persistentHandle;
	}

	// Acquire free buffer, if no one left we need to create a new target.
	if (!pool->free.empty())
	{
		Ref< Buffer > target = pool->free.back();

		pool->free.pop_back();
		pool->acquired.push_back(target);

		return target;
	}
	else
	{
		Ref< Buffer > buffer = m_renderSystem->createBuffer(
			render::BuStructured,
			bufferSize,
			false
		);
		if (buffer)
		{
			void* ptr = buffer->lock();
			if (!ptr)
				return nullptr;
			std::memset(ptr, 0, bufferSize);
			buffer->unlock();

			pool->acquired.push_back(buffer);
		}
		return buffer;
	}
}

void RenderGraphBufferPool::release(Ref< Buffer >& buffer)
{
	T_ANONYMOUS_VAR(Ref< Buffer >)(buffer);
	for (auto& pool : m_pool)
	{
		auto it = std::remove_if(pool.acquired.begin(), pool.acquired.end(), [&](const Buffer* bf) {
			return bf == buffer;
		});
		if (it != pool.acquired.end())
		{
			pool.acquired.erase(it, pool.acquired.end());
			pool.free.push_back(buffer);
			break;
		}
	}
	buffer = nullptr;
}

}
