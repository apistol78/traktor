#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/VertexBufferDynamicVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDynamicVk", VertexBufferDynamicVk, VertexBufferVk)

VertexBufferDynamicVk::VertexBufferDynamicVk(
	uint32_t bufferSize,
	VmaAllocator allocator,
	VmaAllocation allocation,
	VkBuffer vertexBuffer,
	const VkVertexInputBindingDescription& vertexBindingDescription,
	const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
	uint32_t hash
)
:	VertexBufferVk(bufferSize, allocator, allocation, vertexBuffer, vertexBindingDescription, vertexAttributeDescriptions, hash)
,	m_locked(false)
{
}

void* VertexBufferDynamicVk::lock()
{
	void* ptr = nullptr;
	if (vmaMapMemory(m_allocator, m_allocation, &ptr) == VK_SUCCESS)
	{
		m_locked = true;
		return ptr;
	}
	else
		return nullptr;
}

void* VertexBufferDynamicVk::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	return nullptr;
}

void VertexBufferDynamicVk::unlock()
{
	if (m_locked)
	{
		vmaUnmapMemory(m_allocator, m_allocation);
		m_locked = false;
	}
}

	}
}
