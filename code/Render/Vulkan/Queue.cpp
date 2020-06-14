#include "Core/Thread/Acquire.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/Queue.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Queue", Queue, Object)

Ref< Queue > Queue::create(VkDevice device, uint32_t queueIndex)
{
	VkQueue queue;
	vkGetDeviceQueue(device, queueIndex, 0, &queue);
	return new Queue(device, queue, queueIndex);
}

void Queue::submit(const VkSubmitInfo& submitInfo, VkFence signalFence)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	vkQueueSubmit(m_queue, 1, &submitInfo, signalFence);
}

void Queue::submitAndWait(const VkSubmitInfo& submitInfo)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_queue);
}

Queue::Queue(VkDevice device, VkQueue queue, uint32_t queueIndex)
:	m_device(device)
,	m_queue(queue)
,	m_queueIndex(queueIndex)
{
}

	}
}