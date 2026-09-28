/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/StaticVector.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor
{

class Semaphore;
class Thread;

}

namespace traktor::render
{

class Context;
class Queue;

/*!
 * \ingroup Render
 */
class CommandBuffer : public Object
{
	T_RTTI_CLASS;

public:
	/*! Semaphore wait of a submission. */
	struct Wait
	{
		VkSemaphore semaphore = VK_NULL_HANDLE;
		VkPipelineStageFlags stages = 0;
		uint64_t value = 0;	//!< Timeline value; ignored by binary semaphores.
	};

	/*! Semaphore signal of a submission. */
	struct Signal
	{
		VkSemaphore semaphore = VK_NULL_HANDLE;
		uint64_t value = 0;	//!< Timeline value; ignored by binary semaphores.
	};

	virtual ~CommandBuffer();

	bool reset();

	/*! Reserve submission ahead of submitting; cleanups added from now on wait until this command buffer is consumed.
	 * For resources which are released as their use is being recorded.
	 */
	void reserveSubmission();

	/*! Submit with up to four waits and four signals, any mix of binary and timeline semaphores. */
	bool submit(const Wait* waits, uint32_t waitCount, const Signal* signals, uint32_t signalCount);

	bool submit(const StaticVector< VkSemaphore, 2 >& waitSemaphores, const StaticVector< VkPipelineStageFlags, 2 >& waitStageFlags, VkSemaphore signalSemaphore);

	bool submitSignal(VkSemaphore semaphore, uint64_t semaphoreValue);

	bool submitWait(VkSemaphore semaphore, uint64_t semaphoreValue, VkPipelineStageFlags stages);

	bool wait();

	/*! Can only be called after caller has made sure GPU is idle. */
	void externalSynced();

	/*! Queue the command buffer is submitted to. */
	Queue* getQueue() const { return m_queue; }

	/*! Submission epoch; reserved or assigned, 0 if neither. */
	uint64_t getSubmissionEpoch() const { return (m_reservedEpoch != 0) ? m_reservedEpoch : (m_submitted ? m_epoch : 0); }

	operator VkCommandBuffer ();

private:
	friend class Queue;

	Context* m_context = nullptr;
	Queue* m_queue = nullptr;
	VkCommandPool m_commandPool = 0;
	VkCommandBuffer m_commandBuffer = 0;
	VkFence m_inFlight = 0;
	Thread* m_thread = nullptr;
	uint64_t m_epoch = 0;	//!< Submission epoch, valid while submitted.
	uint64_t m_reservedEpoch = 0;	//!< Submission epoch reserved ahead of submitting.
	bool m_submitted = false;

	explicit CommandBuffer(Context* context, Queue* queue, VkCommandPool commandPool, VkCommandBuffer commandBuffer);

	uint64_t beginSubmission();
};

}
