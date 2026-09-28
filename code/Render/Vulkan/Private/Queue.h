/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Thread/CriticalSection.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Vulkan/Private/ApiHeader.h"

#include <thread>

namespace traktor::render
{

class CommandBuffer;
class Context;

/*!
 * \ingroup Render
 */
class Queue : public Object
{
	T_RTTI_CLASS;

public:
	Queue() = delete;

	Queue(const Queue&) = delete;

	virtual ~Queue();

	static Ref< Queue > create(Context* context, uint32_t queueIndex);

	/*! Acquire a command buffer from the calling thread's pool; only the calling thread may record, submit and reset it. */
	Ref< CommandBuffer > acquireCommandBuffer(const wchar_t* const tag);

	/*! Acquire a command buffer from an explicit pool; caller synchronizes the pool. */
	Ref< CommandBuffer > acquireCommandBuffer(const wchar_t* const tag, VkCommandPool commandPool);

	/*! Create a command pool for this queue's family; caller owns the pool. */
	VkCommandPool createCommandPool() const;

	VkResult submit(const VkSubmitInfo& si, VkFence fence);

	VkResult present(const VkPresentInfoKHR& pi);

	uint32_t getQueueIndex() const { return m_queueIndex; }

	/*! True if graphics commands, and thus graphics pipeline stages, are supported by the queue. */
	bool supportsGraphics() const { return (m_queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0; }

private:
	friend class Context;

	Context* m_context;
	VkQueue m_queue;
	uint32_t m_queueIndex;
	VkQueueFlags m_queueFlags = 0;
	Semaphore m_lock;
	uint64_t m_uploadValueWaited = 0;	//!< Upload submission value which work on this queue already waits on.
	CriticalSection m_commandPoolsLock;
	SmallMap< std::thread::id, VkCommandPool > m_commandPools;	//!< Command pool of each thread which has acquired command buffers from this queue.

	explicit Queue(Context* context, VkQueue queue, uint32_t queueIndex, VkQueueFlags queueFlags);
};

}
