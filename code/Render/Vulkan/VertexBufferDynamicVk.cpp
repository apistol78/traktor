#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/VertexBufferDynamicVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDynamicVk", VertexBufferDynamicVk, VertexBufferVk)

VertexBufferDynamicVk::VertexBufferDynamicVk(
	uint32_t bufferSize,
	const VkVertexInputBindingDescription& vertexBindingDescription,
	const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
	uint32_t hash
)
:	VertexBufferVk(bufferSize, vertexBindingDescription, vertexAttributeDescriptions, hash)
,	m_index(0)
{
}

bool VertexBufferDynamicVk::create(VmaAllocator allocator, int32_t inFlightCount)
{
	const uint32_t bufferSize = getBufferSize();
	if (!bufferSize)
		return false;

	m_buffers.resize(inFlightCount);
	for (auto& buffer : m_buffers)
	{
		if (!buffer.create(allocator, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true, true))
			return false;
	}

	return true;
}

void VertexBufferDynamicVk::destroy()
{
	for (auto& buffer : m_buffers)
		buffer.destroy();
	m_buffers.clear();
}

void* VertexBufferDynamicVk::lock()
{
	int32_t next = (m_index + 1) % (int32_t)m_buffers.size();
	return m_buffers[next].lock();
}

void* VertexBufferDynamicVk::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_FATAL_ERROR;
	return nullptr;
}

void VertexBufferDynamicVk::unlock()
{
	int32_t next = (m_index + 1) % (int32_t)m_buffers.size();
	m_buffers[next].unlock();
	m_index = next;
}

VkBuffer VertexBufferDynamicVk::getVkBuffer() const
{
	return m_buffers[m_index];
}

	}
}
