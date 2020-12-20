#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/StructBufferVk.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferVk", StructBufferVk, StructBuffer)

StructBufferVk::StructBufferVk(uint32_t bufferSize)
:	StructBuffer(bufferSize)
,	m_index(0)
{
}

bool StructBufferVk::create(VkDevice logicalDevice, VmaAllocator allocator, int32_t inFlightCount)
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return false;

	m_buffers.resize(inFlightCount);
	for (auto& buffer : m_buffers)
	{
		if (!buffer.create(logicalDevice, allocator, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true, true))
			return false;
	}

	return true;
}

void StructBufferVk::destroy()
{
	for (auto& buffer : m_buffers)
		buffer.destroy();
	m_buffers.clear();
}

void* StructBufferVk::lock()
{
	int32_t next = (m_index + 1) % (int32_t)m_buffers.size();
	return m_buffers[next].lock();
}

void* StructBufferVk::lock(uint32_t structOffset, uint32_t structCount)
{
	T_FATAL_ERROR;
	return nullptr;
}

void StructBufferVk::unlock()
{
	int32_t next = (m_index + 1) % (int32_t)m_buffers.size();
	m_buffers[next].unlock();
	m_index = next;
}

	}
}