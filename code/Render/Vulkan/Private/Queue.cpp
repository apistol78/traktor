/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Acquire.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Queue", Queue, Object)

thread_local VkCommandPool Queue::ms_commandPools[32];

Ref< Queue > Queue::create(Context* context, uint32_t queueIndex)
{
	VkQueue queue;
	T_FATAL_ASSERT(queueIndex < sizeof_array(ms_commandPools));
	vkGetDeviceQueue(context->getLogicalDevice(), queueIndex, 0, &queue);
	return new Queue(context, queue, queueIndex);
}

Ref< CommandBuffer > Queue::acquireCommandBuffer(const wchar_t* const tag)
{
	if (!ms_commandPools[m_queueIndex])
	{
		VkCommandPool commandPool;

		VkCommandPoolCreateInfo cpci = {};
		cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		cpci.queueFamilyIndex = m_queueIndex;
		if (vkCreateCommandPool(m_context->getLogicalDevice(), &cpci, 0, &commandPool) != VK_SUCCESS)
			return nullptr;

		ms_commandPools[m_queueIndex] = commandPool;
	}

	VkCommandBuffer commandBuffer = 0;

	VkCommandBufferAllocateInfo cbai = {};
	cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbai.commandPool = ms_commandPools[m_queueIndex];
	cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbai.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(m_context->getLogicalDevice(), &cbai, &commandBuffer) != VK_SUCCESS)
		return nullptr;

	setObjectDebugName(m_context->getLogicalDevice(), tag, (uint64_t)commandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER);

	VkCommandBufferBeginInfo cbbi = {};
	cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(commandBuffer, &cbbi) != VK_SUCCESS)
		return nullptr;

	return new CommandBuffer(m_context, this, ms_commandPools[m_queueIndex], commandBuffer);
}

VkResult Queue::submit(const VkSubmitInfo& si, VkFence fence)
{
	VkResult result;
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		result = vkQueueSubmit(m_queue, 1, &si, fence);
		T_ASSERT(result == VK_SUCCESS);
	}
	return result;
}

VkResult Queue::present(const VkPresentInfoKHR& pi)
{
	VkResult result;
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		result = vkQueuePresentKHR(m_queue, &pi);
	}
	return result;
}

Queue::Queue(Context* context, VkQueue queue, uint32_t queueIndex)
:	m_context(context)
,	m_queue(queue)
,	m_queueIndex(queueIndex)
{
}

}