/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/StaticVector.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Thread/Acquire.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Queue", Queue, Object)

Queue::~Queue()
{
	// Destroying a pool implicitly frees any command buffer still allocated from it.
	for (const auto& it : m_commandPools)
		vkDestroyCommandPool(m_context->getLogicalDevice(), it.second, nullptr);
	m_commandPools.clear();
}

Ref< Queue > Queue::create(Context* context, uint32_t queueIndex)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(context->getPhysicalDevice(), &queueFamilyCount, nullptr);

	AutoArrayPtr< VkQueueFamilyProperties > queueFamilyProperties(new VkQueueFamilyProperties[queueFamilyCount]);
	vkGetPhysicalDeviceQueueFamilyProperties(context->getPhysicalDevice(), &queueFamilyCount, queueFamilyProperties.ptr());

	const VkQueueFlags queueFlags = (queueIndex < queueFamilyCount) ? queueFamilyProperties[queueIndex].queueFlags : 0;

	VkQueue queue;
	vkGetDeviceQueue(context->getLogicalDevice(), queueIndex, 0, &queue);
	return new Queue(context, queue, queueIndex, queueFlags);
}

Ref< CommandBuffer > Queue::acquireCommandBuffer(const wchar_t* const tag)
{
	// Command pools require external synchronization, thus every thread records from a pool of its own.
	VkCommandPool commandPool = 0;
	{
		T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_commandPoolsLock);
		const std::thread::id threadId = std::this_thread::get_id();
		auto it = m_commandPools.find(threadId);
		if (it != m_commandPools.end())
			commandPool = it->second;
		else
		{
			if ((commandPool = createCommandPool()) == 0)
				return nullptr;
			m_commandPools.insert(threadId, commandPool);
		}
	}
	return acquireCommandBuffer(tag, commandPool);
}

Ref< CommandBuffer > Queue::acquireCommandBuffer(const wchar_t* const tag, VkCommandPool commandPool)
{
	VkCommandBuffer commandBuffer = 0;

	const VkCommandBufferAllocateInfo cbai =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
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
	{
		vkFreeCommandBuffers(m_context->getLogicalDevice(), commandPool, 1, &commandBuffer);
		return nullptr;
	}

	return new CommandBuffer(m_context, this, commandPool, commandBuffer);
}

VkCommandPool Queue::createCommandPool() const
{
	const VkCommandPoolCreateInfo cpci =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		.queueFamilyIndex = m_queueIndex
	};
	VkCommandPool commandPool = 0;
	if (vkCreateCommandPool(m_context->getLogicalDevice(), &cpci, nullptr, &commandPool) != VK_SUCCESS)
		return 0;
	return commandPool;
}

VkResult Queue::submit(const VkSubmitInfo& si, VkFence fence)
{
	// Work on other queues might use uploaded resources, thus waits for uploads; the value is guarded by the graphics queue.
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

	// Append a wait on the upload timeline; the timeline submit info, if any, is amended to cover every wait.
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

Queue::Queue(Context* context, VkQueue queue, uint32_t queueIndex, VkQueueFlags queueFlags)
:	m_context(context)
,	m_queue(queue)
,	m_queueIndex(queueIndex)
,	m_queueFlags(queueFlags)
{
}

}
