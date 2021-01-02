#pragma once

#include "Core/Object.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor
{

class Semaphore;
class Thread;

	namespace render
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

	operator VkCommandBuffer ();

private:
	friend class Queue;

	Context* m_context;
	Queue* m_queue;
	VkCommandBuffer m_commandBuffer;
	VkFence m_inFlight;

	Thread* m_thread;

	explicit CommandBuffer(Context* context, Queue* queue, VkCommandBuffer commandBuffer);
};

	}
}