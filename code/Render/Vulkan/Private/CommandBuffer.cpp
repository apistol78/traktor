/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/Private/CommandBuffer.h"

#include "Core/Log/Log.h"
#include "Core/Thread/ThreadManager.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CommandBuffer", CommandBuffer, Object)

CommandBuffer::~CommandBuffer()
{
	T_FATAL_ASSERT_M(!m_submitted, L"Cannot destroy in-flight command buffer, must wait until finished.");

	// Never submitted; release reservation so cleanups are not held back by it.
	if (m_reservedEpoch != 0)
		m_context->endSubmission(m_reservedEpoch, VK_NULL_HANDLE);

	vkFreeCommandBuffers(
		m_context->getLogicalDevice(),
		m_commandPool,
		1,
		&m_commandBuffer);
	vkDestroyFence(m_context->getLogicalDevice(), m_inFlight, nullptr);
}

bool CommandBuffer::reset()
{
	T_FATAL_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_thread);
	T_FATAL_ASSERT(!m_submitted);

	vkResetCommandBuffer(m_commandBuffer, 0);

	const VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS)
		return false;

	m_submitted = false;
	return true;
}

void CommandBuffer::reserveSubmission()
{
	T_FATAL_ASSERT(!m_submitted);
	T_FATAL_ASSERT(m_reservedEpoch == 0);
	m_reservedEpoch = m_context->beginSubmission(m_inFlight);
}

bool CommandBuffer::submit(const Wait* waits, uint32_t waitCount, const Signal* signals, uint32_t signalCount)
{
	T_FATAL_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_thread);
	T_FATAL_ASSERT(!m_submitted);
	T_FATAL_ASSERT(waitCount <= 4 && signalCount <= 4);
	VkResult result;

	vkEndCommandBuffer(m_commandBuffer);

	StaticVector< VkSemaphore, 4 > waitSemaphores;
	StaticVector< VkPipelineStageFlags, 4 > waitStageFlags;
	StaticVector< uint64_t, 4 > waitValues;
	for (uint32_t i = 0; i < waitCount; ++i)
	{
		waitSemaphores.push_back(waits[i].semaphore);
		waitStageFlags.push_back(waits[i].stages);
		waitValues.push_back(waits[i].value);
	}

	StaticVector< VkSemaphore, 4 > signalSemaphores;
	StaticVector< uint64_t, 4 > signalValues;
	for (uint32_t i = 0; i < signalCount; ++i)
	{
		signalSemaphores.push_back(signals[i].semaphore);
		signalValues.push_back(signals[i].value);
	}

	// Binary semaphores ignore their values, thus one timeline info covers any mix.
	const VkTimelineSemaphoreSubmitInfo timelineInfo = {
		.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreValueCount = (uint32_t)waitValues.size(),
		.pWaitSemaphoreValues = waitValues.c_ptr(),
		.signalSemaphoreValueCount = (uint32_t)signalValues.size(),
		.pSignalSemaphoreValues = signalValues.c_ptr()
	};

	const VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = (waitCount > 0 || signalCount > 0) ? &timelineInfo : nullptr,
		.waitSemaphoreCount = (uint32_t)waitSemaphores.size(),
		.pWaitSemaphores = waitSemaphores.c_ptr(),
		.pWaitDstStageMask = waitStageFlags.c_ptr(),
		.commandBufferCount = 1,
		.pCommandBuffers = &m_commandBuffer,
		.signalSemaphoreCount = (uint32_t)signalSemaphores.size(),
		.pSignalSemaphores = signalSemaphores.c_ptr()
	};

	const uint64_t epoch = beginSubmission();
	if ((result = m_queue->submit(submitInfo, m_inFlight)) != VK_SUCCESS)
	{
		m_context->endSubmission(epoch, m_inFlight);
		log::error << L"Unable to submit command buffer, \"" << getHumanResult(result) << L"\"." << Endl;
		return false;
	}
	m_context->submissionIssued(epoch);

	m_epoch = epoch;
	m_submitted = true;
	return true;
}

bool CommandBuffer::submit(const StaticVector< VkSemaphore, 2 >& waitSemaphores, const StaticVector< VkPipelineStageFlags, 2 >& waitStageFlags, VkSemaphore signalSemaphore)
{
	T_ASSERT(waitSemaphores.size() == waitStageFlags.size());

	StaticVector< Wait, 2 > waits;
	for (uint32_t i = 0; i < (uint32_t)waitSemaphores.size(); ++i)
		waits.push_back({ .semaphore = waitSemaphores[i], .stages = waitStageFlags[i] });

	const Signal signal = { .semaphore = signalSemaphore };
	return submit(waits.c_ptr(), (uint32_t)waits.size(), &signal, (signalSemaphore != VK_NULL_HANDLE) ? 1 : 0);
}

bool CommandBuffer::submitSignal(VkSemaphore semaphore, uint64_t semaphoreValue)
{
	const Signal signal = { .semaphore = semaphore, .value = semaphoreValue };
	return submit(nullptr, 0, &signal, 1);
}

bool CommandBuffer::submitWait(VkSemaphore semaphore, uint64_t semaphoreValue, VkPipelineStageFlags stages)
{
	const Wait wait = { .semaphore = semaphore, .stages = stages, .value = semaphoreValue };
	return submit(&wait, 1, nullptr, 0);
}

bool CommandBuffer::wait()
{
	if (!m_submitted)
		return true;

	const bool result = (vkWaitForFences(m_context->getLogicalDevice(), 1, &m_inFlight, VK_TRUE, UINT64_MAX) == VK_SUCCESS);

	// Fence is reset as the submission ends; until then it might be polled from another thread.
	m_context->endSubmission(m_epoch, m_inFlight);

	m_submitted = false;
	return result;
}

void CommandBuffer::externalSynced()
{
	if (m_submitted)
	{
		m_context->endSubmission(m_epoch, m_inFlight);
		m_submitted = false;
	}
}

CommandBuffer::operator VkCommandBuffer()
{
	T_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_thread);
	return m_commandBuffer;
}

CommandBuffer::CommandBuffer(Context* context, Queue* queue, VkCommandPool commandPool, VkCommandBuffer commandBuffer)
	: m_context(context)
	, m_queue(queue)
	, m_commandPool(commandPool)
	, m_commandBuffer(commandBuffer)
{
	const VkFenceCreateInfo fenceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = 0
	};
	vkCreateFence(m_context->getLogicalDevice(), &fenceCreateInfo, nullptr, &m_inFlight);
	vkResetFences(m_context->getLogicalDevice(), 1, &m_inFlight);

	m_context->setObjectDebugName(T_FILE_LINE_W, (uint64_t)m_inFlight, VK_OBJECT_TYPE_FENCE);

	m_thread = ThreadManager::getInstance().getCurrentThread();
}

uint64_t CommandBuffer::beginSubmission()
{
	// Use reserved submission if any.
	if (m_reservedEpoch != 0)
	{
		const uint64_t epoch = m_reservedEpoch;
		m_reservedEpoch = 0;
		return epoch;
	}
	return m_context->beginSubmission(m_inFlight);
}

}
