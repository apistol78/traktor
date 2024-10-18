/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Thread/ThreadManager.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CommandBuffer", CommandBuffer, Object)

CommandBuffer::~CommandBuffer()
{
	T_FATAL_ASSERT_M(!m_submitted, L"Cannot destroy in-flight command buffer, must wait until finished.");
	vkFreeCommandBuffers(
		m_context->getLogicalDevice(),
		m_commandPool,
		1,
		&m_commandBuffer
	);
	vkDestroyFence(m_context->getLogicalDevice(), m_inFlight, nullptr);
}

bool CommandBuffer::reset()
{
	T_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_thread);
	T_ASSERT(!m_submitted);

	vkResetCommandBuffer(m_commandBuffer, 0);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS)
		return false;

	m_submitted = false;
	return true;
}

bool CommandBuffer::submit(VkSemaphore waitSemaphore, VkPipelineStageFlags waitStageFlags, VkSemaphore signalSemaphore)
{
	T_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_thread);
	T_ASSERT(!m_submitted);
	VkResult result;

	vkEndCommandBuffer(m_commandBuffer);

	VkSubmitInfo si = {};
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	if (waitSemaphore != VK_NULL_HANDLE)
	{
		si.waitSemaphoreCount = 1;
		si.pWaitSemaphores = &waitSemaphore;
		si.pWaitDstStageMask = &waitStageFlags;
	}

	si.commandBufferCount = 1;
	si.pCommandBuffers = &m_commandBuffer;

	if (signalSemaphore != VK_NULL_HANDLE)
	{
		si.signalSemaphoreCount = 1;
		si.pSignalSemaphores = &signalSemaphore;
	}

	if ((result = m_queue->submit(si, m_inFlight)) != VK_SUCCESS)
	{
		log::error << L"Unable to submit command buffer, \"" << getHumanResult(result) << L"\"." << Endl;
		return false;
	}

	m_submitted = true;
	return true;
}

bool CommandBuffer::wait()
{
	T_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_thread);

	if (!m_submitted)
		return true;

	const bool result = (vkWaitForFences(m_context->getLogicalDevice(), 1, &m_inFlight, VK_TRUE, UINT64_MAX) == VK_SUCCESS);
	vkResetFences(m_context->getLogicalDevice(), 1, &m_inFlight);

	m_submitted = false;
	return result;
}

bool CommandBuffer::submitAndWait()
{
	if (!submit(VK_NULL_HANDLE, 0, VK_NULL_HANDLE))
		return false;
	if (!wait())
		return false;
	return true;
}

void CommandBuffer::externalSynced()
{
	m_submitted = false;
}

CommandBuffer::operator VkCommandBuffer ()
{
	T_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_thread);
	return m_commandBuffer;
}

CommandBuffer::CommandBuffer(Context* context, Queue* queue, VkCommandPool commandPool, VkCommandBuffer commandBuffer)
:	m_context(context)
,	m_queue(queue)
,	m_commandPool(commandPool)
,	m_commandBuffer(commandBuffer)
{
	VkFenceCreateInfo fci = {};
	fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fci.flags = 0;
	vkCreateFence(m_context->getLogicalDevice(), &fci, nullptr, &m_inFlight);
	vkResetFences(m_context->getLogicalDevice(), 1, &m_inFlight);

	setObjectDebugName(m_context->getLogicalDevice(), T_FILE_LINE_W, (uint64_t)m_inFlight, VK_OBJECT_TYPE_FENCE);

	m_thread = ThreadManager::getInstance().getCurrentThread();
}

}
