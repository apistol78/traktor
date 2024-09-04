/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Spray/EmitterInstanceGPU.h"
#include "Spray/GPUBufferPool.h"
#include "Spray/Point.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.GPUBufferPool", GPUBufferPool, Object)

GPUBufferPool::GPUBufferPool(render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

bool GPUBufferPool::allocBuffers(uint32_t capacity, Ref< render::Buffer >& outHeadBuffer, Ref< render::Buffer >& outPointBuffer)
{
	outHeadBuffer = nullptr;
	outPointBuffer = nullptr;

	for (auto it = m_pointBuffers.begin(); it != m_pointBuffers.end(); ++it)
	{
		const size_t candidateCapacity = (*it)->getBufferSize() / sizeof(Point);
		if (candidateCapacity >= capacity)
		{
			outPointBuffer = *it;
			m_pointBuffers.erase(it);
			break;
		}
	}

	if (outPointBuffer == nullptr)
	{
		outPointBuffer = m_renderSystem->createBuffer(render::BuStructured, capacity * sizeof(Point), false);
		if (outPointBuffer == nullptr)
			return false;
	}

	if (!m_headBuffers.empty())
	{
		outHeadBuffer = m_headBuffers.back();
		m_headBuffers.pop_back();
	}
	else
	{
		outHeadBuffer = m_renderSystem->createBuffer(render::BuStructured | render::BuIndirect, sizeof(Head), false);
		if (outHeadBuffer == nullptr)
		{
			m_pointBuffers.push_back(outPointBuffer);
			outPointBuffer = nullptr;
			return false;
		}
	}

	T_FATAL_ASSERT(outHeadBuffer != nullptr);
	T_FATAL_ASSERT(outPointBuffer != nullptr);
	return true;
}

void GPUBufferPool::freeBuffers(Ref< render::Buffer >& inoutHeadBuffer, Ref< render::Buffer >& inoutPointBuffer)
{
	T_FATAL_ASSERT(inoutHeadBuffer != nullptr);
	T_FATAL_ASSERT(inoutPointBuffer != nullptr);

	m_headBuffers.push_back(inoutHeadBuffer);
	m_pointBuffers.push_back(inoutPointBuffer);

	inoutHeadBuffer = nullptr;
	inoutPointBuffer = nullptr;
}

}
