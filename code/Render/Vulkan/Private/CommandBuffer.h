/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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
	virtual ~CommandBuffer();

	bool reset();

	/*! Reserve submission ahead of submitting.
	 *
	 * Cleanups added from now on are held back until this command buffer has been
	 * consumed, as if it had been submitted already; for resources which are released
	 * as their use is being recorded. \sa Context::addDeferredCleanup
	 */
	void reserveSubmission();

	bool submit(const StaticVector< VkSemaphore, 2 >& waitSemaphores, const StaticVector< VkPipelineStageFlags, 2 >& waitStageFlags, VkSemaphore signalSemaphore);

	bool submitSignal(VkSemaphore semaphore, uint64_t semaphoreValue);

	bool submitWait(VkSemaphore semaphore, uint64_t semaphoreValue, VkPipelineStageFlags stages);

	bool wait();

	bool submitAndWait();

	/*! Can only be called after caller has made sure GPU is idle. */
	void externalSynced();

	operator VkCommandBuffer ();

private:
	friend class Queue;

	Context* m_context = nullptr;
	Queue* m_queue = nullptr;
	VkCommandPool m_commandPool = 0;
	VkCommandBuffer m_commandBuffer = 0;
	VkFence m_inFlight = 0;
	Thread* m_thread = nullptr;
	uint64_t m_epoch = 0;	//!< Submission epoch, valid while submitted; \sa Context::beginSubmission.
	uint64_t m_reservedEpoch = 0;	//!< Submission epoch reserved ahead of submitting; \sa reserveSubmission.
	bool m_submitted = false;

	explicit CommandBuffer(Context* context, Queue* queue, VkCommandPool commandPool, VkCommandBuffer commandBuffer);

	uint64_t beginSubmission();
};

}