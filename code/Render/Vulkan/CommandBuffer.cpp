#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadManager.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/CommandBuffer.h"
#include "Render/Vulkan/Context.h"
#include "Render/Vulkan/Queue.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CommandBuffer", CommandBuffer, Object)

CommandBuffer::~CommandBuffer()
{
	vkFreeCommandBuffers(
		m_context->getLogicalDevice(),
		m_queue->ms_commandPool,
		1,
		&m_commandBuffer
	);
	vkDestroyFence(m_context->getLogicalDevice(), m_inFlight, nullptr);
}

bool CommandBuffer::reset()
{
	T_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_thread);

	vkResetCommandBuffer(m_commandBuffer, 0);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS)
		return false;

	return true;
}

bool CommandBuffer::submit(VkSemaphore waitSemaphore, VkPipelineStageFlags waitStageFlags, VkSemaphore signalSemaphore)
{
	T_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_thread);

	vkEndCommandBuffer(m_commandBuffer);

	vkResetFences(m_context->getLogicalDevice(), 1, &m_inFlight);

	VkSubmitInfo si = {};
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	if (waitSemaphore != VK_NULL_HANDLE)
	{
		si.waitSemaphoreCount = 1;
		si.pWaitSemaphores = &waitSemaphore;
	}

	si.pWaitDstStageMask = &waitStageFlags;
	si.commandBufferCount = 1;
	si.pCommandBuffers = &m_commandBuffer;

	if (signalSemaphore != VK_NULL_HANDLE)
	{
		si.signalSemaphoreCount = 1;
		si.pSignalSemaphores = &signalSemaphore;
	}

	bool result;
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_queue->m_lock);
		result = (bool)(vkQueueSubmit(*m_queue, 1, &si, m_inFlight) == VK_SUCCESS);
	}
	return result;
}

bool CommandBuffer::wait()
{
	const uint64_t timeOut = 5 * 60 * 1000ull * 1000ull * 1000ull;
    return (bool)(vkWaitForFences(m_context->getLogicalDevice(), 1, &m_inFlight, VK_TRUE, timeOut) == VK_SUCCESS);
}

bool CommandBuffer::submitAndWait()
{
	if (!submit(VK_NULL_HANDLE, 0, VK_NULL_HANDLE))
		return false;
	if (!wait())
		return false;
	return true;
}

CommandBuffer::operator VkCommandBuffer ()
{
	T_ASSERT(ThreadManager::getInstance().getCurrentThread() == m_thread);
	return m_commandBuffer;
}

CommandBuffer::CommandBuffer(Context* context, Queue* queue, VkCommandBuffer commandBuffer)
:	m_context(context)
,	m_queue(queue)
,	m_commandBuffer(commandBuffer)
{
	VkFenceCreateInfo fci = {};
	fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	vkCreateFence(m_context->getLogicalDevice(), &fci, nullptr, &m_inFlight);	

	m_thread = ThreadManager::getInstance().getCurrentThread();
}

	}
}
