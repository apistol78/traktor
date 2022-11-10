/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
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

class CommandBuffer : public Object
{
	T_RTTI_CLASS;

public:
	virtual ~CommandBuffer();

	bool reset();

	bool submit(VkSemaphore waitSemaphore, VkPipelineStageFlags waitStageFlags, VkSemaphore signalSemaphore);

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
	bool m_submitted = false;

	explicit CommandBuffer(Context* context, Queue* queue, VkCommandPool commandPool, VkCommandBuffer commandBuffer);
};

}