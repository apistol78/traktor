#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class Queue : public Object
{
	T_RTTI_CLASS;

public:
	Queue() = delete;

	Queue(const Queue&) = delete;

	static Ref< Queue > create(VkDevice device, uint32_t queueIndex);

	void submit(const VkSubmitInfo& submitInfo, VkFence signalFence = VK_NULL_HANDLE);

	void submitAndWait(const VkSubmitInfo& submitInfo);

	uint32_t getQueueIndex() const { return m_queueIndex; }

private:
	VkDevice m_device;
	VkQueue m_queue;
	uint32_t m_queueIndex;
	Semaphore m_lock;

	Queue(VkDevice device, VkQueue queue, uint32_t queueIndex);
};

	}
}
