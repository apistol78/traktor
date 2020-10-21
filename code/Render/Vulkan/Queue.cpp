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
	VkFence fence;

	// Get opaque queue from index.
	vkGetDeviceQueue(device, queueIndex, 0, &queue);

	// Create fence used for synchronization of "submit'n'wait" operations.
	VkFenceCreateInfo fci = {};
	fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fci.pNext = nullptr;
	fci.flags = 0;
	vkCreateFence(device, &fci, nullptr, &fence);

	return new Queue(device, queue, queueIndex, fence);
}

bool Queue::submit(const VkSubmitInfo& submitInfo, VkFence signalFence)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return (bool)(vkQueueSubmit(m_queue, 1, &submitInfo, signalFence) == VK_SUCCESS);
}

bool Queue::submitAndWait(const VkSubmitInfo& submitInfo)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// Submit command buffer to queue, signal fence when it's been executed.
	if (vkQueueSubmit(m_queue, 1, &submitInfo, m_fence) != VK_SUCCESS)
		return false;

	// Wait until command buffer has been executed.
	vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX);
	vkResetFences(m_device, 1, &m_fence);
	return true;
}

Queue::Queue(VkDevice device, VkQueue queue, uint32_t queueIndex, VkFence fence)
:	m_device(device)
,	m_queue(queue)
,	m_queueIndex(queueIndex)
,	m_fence(fence)
{
}

	}
}