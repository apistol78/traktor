#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/StructBufferVk.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferVk", StructBufferVk, StructBuffer)

StructBufferVk::StructBufferVk(
	uint32_t bufferSize,
	VmaAllocator allocator,
	VmaAllocation allocation,
	VkBuffer storageBuffer
)
:	StructBuffer(bufferSize)
,	m_allocator(allocator)
,	m_allocation(allocation)
,	m_storageBuffer(storageBuffer)
,	m_locked(false)
{
}

void StructBufferVk::destroy()
{
}

void* StructBufferVk::lock()
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

void* StructBufferVk::lock(uint32_t structOffset, uint32_t structCount)
{
	return nullptr;
}

void StructBufferVk::unlock()
{
	if (m_locked)
	{
		vmaUnmapMemory(m_allocator, m_allocation);
		m_locked = false;
	}
}

	}
}