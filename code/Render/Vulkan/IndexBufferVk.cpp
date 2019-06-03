#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/IndexBufferVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferVk", IndexBufferVk, IndexBuffer)

IndexBufferVk::IndexBufferVk(
	IndexType indexType,
	uint32_t bufferSize,
	VmaAllocator allocator,
	VmaAllocation allocation,
	VkBuffer indexBuffer
)
:	IndexBuffer(indexType, bufferSize)
,	m_allocator(allocator)
,	m_allocation(allocation)
,	m_indexBuffer(indexBuffer)
,	m_locked(false)
{
}

void IndexBufferVk::destroy()
{
}

void* IndexBufferVk::lock()
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

void IndexBufferVk::unlock()
{
	if (m_locked)
	{
		vmaUnmapMemory(m_allocator, m_allocation);
		m_locked = false;
	}
}

	}
}
