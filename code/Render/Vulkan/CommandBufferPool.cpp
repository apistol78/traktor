#include "Core/Thread/Acquire.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/CommandBufferPool.h"
#include "Render/Vulkan/Queue.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CommandBufferPool", CommandBufferPool, Object)

CommandBufferPool::~CommandBufferPool()
{
	// \fixme!!
	// vkDestroyCommandPool(m_device, m_commandPool, nullptr);
}

Ref< CommandBufferPool > CommandBufferPool::create(VkDevice device, Queue* queue)
{
	return new CommandBufferPool(device, queue);
}

VkCommandBuffer CommandBufferPool::acquire()
{
	// Get command pool for calling thread, lazy create if no pool has been created.
	VkCommandPool commandPool = (VkCommandPool)m_commandPool.get();
	if (!commandPool)
	{
		VkCommandPoolCreateInfo cpci = {};
		cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		cpci.queueFamilyIndex = m_queue->getQueueIndex();
		if (vkCreateCommandPool(m_device, &cpci, 0, &commandPool) != VK_SUCCESS)
			return 0;
		m_commandPool.set((void*)commandPool);
	}

	// Allocate command buffer from pool.
	VkCommandBuffer commandBuffer = 0;

	VkCommandBufferAllocateInfo cbai = {};
	cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbai.commandPool = commandPool;
	cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbai.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(m_device, &cbai, &commandBuffer) != VK_SUCCESS)
		return 0;
	
	return commandBuffer;
}

VkCommandBuffer CommandBufferPool::acquireAndBegin()
{
	VkCommandBuffer commandBuffer = acquire();
	if (!commandBuffer)
		return 0;

	VkCommandBufferBeginInfo cbbi = {};
	cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &cbbi);

	return commandBuffer;
}

void CommandBufferPool::release(VkCommandBuffer& inoutCommandBuffer)
{
	VkCommandPool commandPool = (VkCommandPool)m_commandPool.get();
	T_FATAL_ASSERT(commandPool != 0);

	vkFreeCommandBuffers(m_device, commandPool, 1, &inoutCommandBuffer);
	inoutCommandBuffer = 0;
}

CommandBufferPool::CommandBufferPool(VkDevice device, Queue* queue)
:	m_device(device)
,	m_queue(queue)
{
}

	}
}