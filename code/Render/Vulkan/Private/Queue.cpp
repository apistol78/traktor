/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/StaticVector.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadLocal.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
{
	namespace
	{

ThreadLocal s_commandPools;

VkCommandPool getCommandPool(VkDevice logicalDevice, uint32_t queueIndex)
{
	VkCommandPool* commandPools = (VkCommandPool*)s_commandPools.get();
	if (!commandPools)
	{
		commandPools = new VkCommandPool[32];
		for (int32_t i = 0; i < 32; ++i)
			commandPools[i] = 0;
		s_commandPools.set(commandPools);
	}
	T_FATAL_ASSERT(queueIndex < 32);
	if (commandPools[queueIndex] == 0)
	{
		const VkCommandPoolCreateInfo cpci =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
			.queueFamilyIndex = queueIndex
		};
		VkCommandPool commandPool;
		if (vkCreateCommandPool(logicalDevice, &cpci, 0, &commandPool) != VK_SUCCESS)
			return 0;
		commandPools[queueIndex] = commandPool;
	}
	return commandPools[queueIndex];
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Queue", Queue, Object)

Ref< Queue > Queue::create(Context* context, uint32_t queueIndex)
{
	VkQueue queue;
	vkGetDeviceQueue(context->getLogicalDevice(), queueIndex, 0, &queue);
	return new Queue(context, queue, queueIndex);
}

Ref< CommandBuffer > Queue::acquireCommandBuffer(const wchar_t* const tag)
{
	VkCommandBuffer commandBuffer = 0;

	const VkCommandBufferAllocateInfo cbai =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = getCommandPool(m_context->getLogicalDevice(), m_queueIndex),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	if (vkAllocateCommandBuffers(m_context->getLogicalDevice(), &cbai, &commandBuffer) != VK_SUCCESS)
		return nullptr;

	m_context->setObjectDebugName(tag, (uint64_t)commandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER);

	const VkCommandBufferBeginInfo cbbi =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	if (vkBeginCommandBuffer(commandBuffer, &cbbi) != VK_SUCCESS)
		return nullptr;

	return new CommandBuffer(m_context, this, cbai.commandPool, commandBuffer);
}

VkResult Queue::submit(const VkSubmitInfo& si, VkFence fence)
{
	// Uploads are submitted to the graphics queue and other work is no longer held back
	// until they have been consumed; work on any other queue might consume uploaded
	// resources thus has to wait for uploads submitted ahead of it. Uploads are dequeued,
	// recorded and submitted with the graphics queue held, reading the value with it held
	// thus ensures every dequeued upload has also been submitted and is waited upon here.
	// \sa Context::performUploads
	uint64_t uploadValue = 0;
	Queue* graphicsQueue = m_context->getGraphicsQueue();
	if (graphicsQueue != this)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(graphicsQueue->m_lock);
		uploadValue = m_context->getUploadValue();
	}

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// A semaphore wait also orders all later work on the queue, thus only wait once per upload.
	if (uploadValue <= m_uploadValueWaited)
	{
		const VkResult result = vkQueueSubmit(m_queue, 1, &si, fence);
		T_ASSERT(result == VK_SUCCESS);
		return result;
	}

	// Append a wait on the upload timeline to the waits of the submission; the timeline
	// submit info, if any, must be amended as it has to cover every wait.
	const VkTimelineSemaphoreSubmitInfo* tsi = nullptr;
	for (const VkBaseInStructure* it = (const VkBaseInStructure*)si.pNext; it != nullptr; it = it->pNext)
	{
		T_FATAL_ASSERT_M(it->sType == VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO, L"Unsupported submit info extension.");
		tsi = (const VkTimelineSemaphoreSubmitInfo*)it;
	}

	StaticVector< VkSemaphore, 8 > waitSemaphores;
	StaticVector< VkPipelineStageFlags, 8 > waitStageFlags;
	StaticVector< uint64_t, 8 > waitValues;
	for (uint32_t i = 0; i < si.waitSemaphoreCount; ++i)
	{
		waitSemaphores.push_back(si.pWaitSemaphores[i]);
		waitStageFlags.push_back(si.pWaitDstStageMask[i]);
		waitValues.push_back((tsi != nullptr && i < tsi->waitSemaphoreValueCount) ? tsi->pWaitSemaphoreValues[i] : 0);
	}
	waitSemaphores.push_back(m_context->getUploadSemaphore());
	waitStageFlags.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	waitValues.push_back(uploadValue);

	const VkTimelineSemaphoreSubmitInfo timelineInfo = {
		.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreValueCount = (uint32_t)waitValues.size(),
		.pWaitSemaphoreValues = waitValues.c_ptr(),
		.signalSemaphoreValueCount = (tsi != nullptr) ? tsi->signalSemaphoreValueCount : 0,
		.pSignalSemaphoreValues = (tsi != nullptr) ? tsi->pSignalSemaphoreValues : nullptr
	};

	VkSubmitInfo usi = si;
	usi.pNext = &timelineInfo;
	usi.waitSemaphoreCount = (uint32_t)waitSemaphores.size();
	usi.pWaitSemaphores = waitSemaphores.c_ptr();
	usi.pWaitDstStageMask = waitStageFlags.c_ptr();

	const VkResult result = vkQueueSubmit(m_queue, 1, &usi, fence);
	T_ASSERT(result == VK_SUCCESS);
	if (result == VK_SUCCESS)
		m_uploadValueWaited = uploadValue;
	return result;
}

VkResult Queue::present(const VkPresentInfoKHR& pi)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	const VkResult result = vkQueuePresentKHR(m_queue, &pi);
	return result;
}

Queue::Queue(Context* context, VkQueue queue, uint32_t queueIndex)
:	m_context(context)
,	m_queue(queue)
,	m_queueIndex(queueIndex)
{
}

}