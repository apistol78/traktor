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

Ref< Buffer > RenderGraphBufferPool::acquire(uint32_t elementCount, uint32_t elementSize)
{
	auto it = m_pool.find({ elementCount, elementSize });
	if (it != m_pool.end() && !it->second.empty())
	{
		Ref< Buffer > buffer = it->second.back();
		it->second.pop_back();
		return buffer;
	}

	Ref< Buffer > buffer = m_renderSystem->createBuffer(
		render::BuStructured,
		elementCount,
		elementSize,
		false
	);
	if (!buffer)
		return nullptr;

	void* ptr = buffer->lock();
	if (!ptr)
		return nullptr;

	std::memset(ptr, 0, elementCount * elementSize);
	buffer->unlock();

	return buffer;
}

void RenderGraphBufferPool::release(Ref< Buffer >& buffer)
{
	m_pool[{ buffer->getElementCount(), buffer->getElementSize() }].push_back(buffer);
	buffer = nullptr;
}

}
