#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/UtilitiesVk.h"
#include "Render/Vulkan/VertexBufferStaticVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferStaticVk", VertexBufferStaticVk, VertexBufferVk)

VertexBufferStaticVk::VertexBufferStaticVk(
	VkDevice logicalDevice,
	VkCommandPool setupCommandPool,
	VkQueue setupQueue,
	uint32_t bufferSize,
	VmaAllocator allocator,
	VmaAllocation allocation,
	VkBuffer vertexBuffer,
	const VkVertexInputBindingDescription& vertexBindingDescription,
	const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
	uint32_t hash
)
:	VertexBufferVk(bufferSize, allocator, allocation, vertexBuffer, vertexBindingDescription, vertexAttributeDescriptions, hash)
,	m_logicalDevice(logicalDevice)
,	m_setupCommandPool(setupCommandPool)
,	m_setupQueue(setupQueue)
,	m_stagingBufferAllocation(0)
,	m_stagingBuffer(0)
{
}

void* VertexBufferStaticVk::lock()
{
	if (m_stagingBufferAllocation != 0)
		return nullptr;

	// Create staging buffer.
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = getBufferSize();
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	if (vmaCreateBuffer(m_allocator, &bufferInfo, &aci, &m_stagingBuffer, &m_stagingBufferAllocation, nullptr) != VK_SUCCESS)
		return nullptr;	

	// Get pointer to staging buffer.
	uint8_t* data = nullptr;
	if (vmaMapMemory(m_allocator, m_stagingBufferAllocation, (void**)&data) != VK_SUCCESS)
		return nullptr;

	return data;
}

void* VertexBufferStaticVk::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	return nullptr;
}

void VertexBufferStaticVk::unlock()
{
	if (m_stagingBufferAllocation == 0)
		return;

	// Unmap staging buffer.
	vmaUnmapMemory(m_allocator, m_stagingBufferAllocation);

	// Copy staging buffer into device buffer
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(m_logicalDevice, m_setupCommandPool);

	VkBufferCopy bufferCopy = {};
	bufferCopy.size = getBufferSize();
	vkCmdCopyBuffer(
		commandBuffer,
		m_stagingBuffer,
		m_vertexBuffer,
		1,
		&bufferCopy
	);

	endSingleTimeCommands(m_logicalDevice, m_setupCommandPool, commandBuffer, m_setupQueue);

	// Free staging buffer.
	vkDestroyBuffer(m_logicalDevice, m_stagingBuffer, 0);
	m_stagingBuffer = 0;

	vmaFreeMemory(m_allocator, m_stagingBufferAllocation);
	m_stagingBufferAllocation = 0;
}

	}
}
