/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/BufferPooledVk.h"

#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"

namespace traktor::render
{
namespace
{

// Frames are built one ahead of rendering, thus frames reading a retired allocation are submitted within this many frames.
const uint64_t c_retiredFrames = 2;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferPooledVk", BufferPooledVk, BufferVk)

BufferPooledVk::BufferPooledVk(Context* context, uint32_t bufferSize, uint32_t& instances)
	: BufferVk(context, bufferSize, instances)
{
}

BufferPooledVk::~BufferPooledVk()
{
	m_retired.push_back(m_current);
	for (auto allocation : m_retired)
	{
		if (allocation)
		{
			safeDestroy(allocation->buffer);
			delete allocation;
		}
	}
	m_retired.clear();
	m_current = nullptr;
	safeDestroy(m_stageBuffer);
	m_context = nullptr;
}

bool BufferPooledVk::create(uint32_t usageBits)
{
	if (!getBufferSize())
		return false;

	m_usageBits = usageBits | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	m_current = createAllocation();
	return m_current != nullptr;
}

void BufferPooledVk::destroy()
{
	// Only relinquish ownership; views handed out must stay valid until rendered, the destructor tears down.
}

void* BufferPooledVk::lock()
{
	T_FATAL_ASSERT(m_stageBuffer == nullptr);

	m_stageBuffer = new ApiBuffer(m_context);
	if (!m_stageBuffer->create(getBufferSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true))
	{
		safeDestroy(m_stageBuffer);
		return nullptr;
	}

	return m_stageBuffer->lock();
}

void BufferPooledVk::unlock()
{
	m_stageBuffer->unlock();

	// Copied into the allocation current now; the buffer is kept alive until the copy is recorded.
	Ref< Buffer > self = this;
	Ref< ApiBuffer > stageBuffer = m_stageBuffer;
	Ref< ApiBuffer > buffer = m_current->buffer;
	m_stageBuffer = nullptr;

	m_context->addDeferredUpload(
		[self, stageBuffer, buffer](Context* cx, CommandBuffer* commandBuffer) mutable {
			const VkBufferCopy bc = {
				.size = buffer->getSize()
			};
			vkCmdCopyBuffer(
				*commandBuffer,
				*stageBuffer,
				*buffer,
				1,
				&bc);
			safeDestroy(stageBuffer);
		},
		getBufferSize());
}

const IBufferView* BufferPooledVk::getBufferView() const
{
	return &m_current->bufferView;
}

void BufferPooledVk::nextFrame()
{
	const uint64_t submittedFrames = m_context->getSubmittedFrameCount();

	m_current->retiredFrame = submittedFrames;
	m_retired.push_back(m_current);

	// Allocations are retired in order, thus when the oldest is still read so are all others.
	if (isFree(m_retired.front(), submittedFrames))
	{
		m_current = m_retired.front();
		m_retired.erase(m_retired.begin());
		return;
	}

	m_current = createAllocation();
	if (!m_current)
	{
		log::error << L"Unable to grow buffer pool; allocation rewritten while in use." << Endl;
		m_current = m_retired.back();
		m_retired.pop_back();
	}
}

BufferPooledVk::Allocation* BufferPooledVk::createAllocation()
{
	const uint32_t bufferSize = getBufferSize();

	Ref< ApiBuffer > buffer = new ApiBuffer(m_context);
	if (!buffer->create(bufferSize, m_usageBits, false, true))
		return nullptr;

	Allocation* allocation = new Allocation();
	allocation->buffer = buffer;
	allocation->bufferView = BufferViewVk(*buffer, 0, bufferSize, bufferSize);
	return allocation;
}

bool BufferPooledVk::isFree(const Allocation* allocation, uint64_t submittedFrames) const
{
	const uint64_t lastFrame = allocation->retiredFrame + c_retiredFrames;
	if (submittedFrames < lastFrame)
		return false;
	return m_context->getCompletedEpoch() >= m_context->getSubmittedFrameEpoch(lastFrame);
}

}
