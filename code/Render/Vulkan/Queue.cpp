#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/Context.h"
#include "Render/Vulkan/CommandBuffer.h"
#include "Render/Vulkan/Queue.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Queue", Queue, Object)

thread_local VkCommandPool Queue::ms_commandPool;

Ref< Queue > Queue::create(Context* context, uint32_t queueIndex)
{
	VkQueue queue;
	vkGetDeviceQueue(context->getLogicalDevice(), queueIndex, 0, &queue);
	return new Queue(context, queue, queueIndex);
}

Ref< CommandBuffer > Queue::acquireCommandBuffer()
{
	if (!ms_commandPool)
	{
		VkCommandPool commandPool;

		VkCommandPoolCreateInfo cpci = {};
		cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		cpci.queueFamilyIndex = m_queueIndex;
		if (vkCreateCommandPool(m_context->getLogicalDevice(), &cpci, 0, &commandPool) != VK_SUCCESS)
			return nullptr;

		ms_commandPool = commandPool;
	}

	VkCommandBuffer commandBuffer = 0;

	VkCommandBufferAllocateInfo cbai = {};
	cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbai.commandPool = ms_commandPool;
	cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbai.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(m_context->getLogicalDevice(), &cbai, &commandBuffer) != VK_SUCCESS)
		return nullptr;

	VkCommandBufferBeginInfo cbbi = {};
	cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(commandBuffer, &cbbi) != VK_SUCCESS)
		return nullptr;

	return new CommandBuffer(m_context, this, commandBuffer);
}

Queue::Queue(Context* context, VkQueue queue, uint32_t queueIndex)
:	m_context(context)
,	m_queue(queue)
,	m_queueIndex(queueIndex)
{
}

	}
}