#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/CommandBufferPool.h"
#include "Render/Vulkan/Queue.h"
#include "Render/Vulkan/UtilitiesVk.h"
#include "Render/Vulkan/VertexBufferStaticVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferStaticVk", VertexBufferStaticVk, VertexBufferVk)

VertexBufferStaticVk::VertexBufferStaticVk(
	VkDevice logicalDevice,
	Queue* graphicsQueue,
	CommandBufferPool* graphicsCommandPool,
	VmaAllocator allocator,
	uint32_t bufferSize,
	const VkVertexInputBindingDescription& vertexBindingDescription,
	const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
	uint32_t hash
)
:	VertexBufferVk(bufferSize, vertexBindingDescription, vertexAttributeDescriptions, hash)
,	m_logicalDevice(logicalDevice)
,	m_graphicsQueue(graphicsQueue)
,	m_graphicsCommandPool(graphicsCommandPool)
,	m_allocator(allocator)
{
}

bool VertexBufferStaticVk::create()
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return false;

	if (!m_deviceBuffer.create(m_logicalDevice, m_allocator, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, false, true))
		return false;

	return true;
}

void VertexBufferStaticVk::destroy()
{
	m_deviceBuffer.destroy();
	m_stageBuffer.destroy();
}

void* VertexBufferStaticVk::lock()
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return nullptr;

	if (!m_stageBuffer.create(m_logicalDevice, m_allocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true))
		return nullptr;

	return m_stageBuffer.lock();
}

void* VertexBufferStaticVk::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_FATAL_ERROR;
	return nullptr;
}

void VertexBufferStaticVk::unlock()
{
	m_stageBuffer.unlock();

	// Copy staging buffer into vertex buffer.
	VkCommandBuffer commandBuffer = m_graphicsCommandPool->acquireAndBegin();

	VkBufferCopy bc = {};
	bc.size = getBufferSize();
	vkCmdCopyBuffer(
		commandBuffer,
		m_stageBuffer,
		m_deviceBuffer,
		1,
		&bc
	);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo si = {};
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.commandBufferCount = 1;
	si.pCommandBuffers = &commandBuffer;
	m_graphicsQueue->submitAndWait(si);

	m_graphicsCommandPool->release(commandBuffer);

	// Free staging buffer.
	m_stageBuffer.destroy();
}

VkBuffer VertexBufferStaticVk::getVkBuffer() const
{
	return m_deviceBuffer;
}

	}
}
