#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadLocal.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class Queue;

class CommandBufferPool : public Object
{
	T_RTTI_CLASS;

public:
	CommandBufferPool() = delete;

	CommandBufferPool(const CommandBufferPool&) = delete;

	virtual ~CommandBufferPool();

	static Ref< CommandBufferPool > create(VkDevice device, Queue* queue);

	VkCommandBuffer acquire();

	VkCommandBuffer acquireAndBegin();

	void release(VkCommandBuffer& inoutCommandBuffer);

private:
	VkDevice m_device;
	Ref< Queue > m_queue;
	// VkCommandPool m_commandPool;
	ThreadLocal m_commandPool;
	Semaphore m_lock;

	CommandBufferPool(VkDevice device, Queue* queue);
};

	}
}