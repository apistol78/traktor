#pragma once

#include <atomic>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class CommandBuffer;
class Context;

class Queue : public Object
{
	T_RTTI_CLASS;

public:
	Queue() = delete;

	Queue(const Queue&) = delete;

	static Ref< Queue > create(Context* context, uint32_t queueIndex);

	Ref< CommandBuffer > acquireCommandBuffer();

	uint32_t getQueueIndex() const { return m_queueIndex; }

	operator VkQueue () { return m_queue; }

private:
	friend class CommandBuffer;

	Context* m_context;
	VkQueue m_queue;
	uint32_t m_queueIndex;
	Semaphore m_lock;
	static thread_local VkCommandPool ms_commandPool;

	Queue(Context* context, VkQueue queue, uint32_t queueIndex);
};

	}
}
