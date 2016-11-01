#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/IndexBufferVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferVk", IndexBufferVk, IndexBuffer)

IndexBufferVk::IndexBufferVk(IndexType indexType, uint32_t bufferSize, VkDevice device, VkBuffer indexBuffer, VkDeviceMemory indexBufferMemory)
:	IndexBuffer(indexType, bufferSize)
,	m_device(device)
,	m_indexBuffer(indexBuffer)
,	m_indexBufferMemory(indexBufferMemory)
{
}

void IndexBufferVk::destroy()
{
}

void* IndexBufferVk::lock()
{
	void* ptr = 0;
	return (vkMapMemory(m_device, m_indexBufferMemory, 0, VK_WHOLE_SIZE, 0, &ptr) == VK_SUCCESS) ? ptr : 0;
}
	
void IndexBufferVk::unlock()
{
	vkUnmapMemory(m_device, m_indexBufferMemory);
	vkBindBufferMemory(m_device, m_indexBuffer, m_indexBufferMemory, 0);
}

	}
}
