/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <atomic>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class CommandBuffer;
class Context;

class Queue : public Object
{
	T_RTTI_CLASS;

public:
	Queue() = delete;

	Queue(const Queue&) = delete;

	static Ref< Queue > create(Context* context, uint32_t queueIndex);

	Ref< CommandBuffer > acquireCommandBuffer(const wchar_t* const tag);

	VkResult submit(const VkSubmitInfo& si, VkFence fence);

	VkResult present(const VkPresentInfoKHR& pi);

	uint32_t getQueueIndex() const { return m_queueIndex; }

private:
	friend class Context;

	Context* m_context;
	VkQueue m_queue;
	uint32_t m_queueIndex;
	Semaphore m_lock;
	static thread_local VkCommandPool ms_commandPools[32];

	Queue(Context* context, VkQueue queue, uint32_t queueIndex);
};

}
