#include "Render/Vulkan/VertexBufferVk.h"
#if defined(_WIN32)
#	include "Render/Vulkan/Win32/ApiLoader.h"
#endif

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferVk", VertexBufferVk, VertexBuffer)

VertexBufferVk::VertexBufferVk(
	uint32_t bufferSize,
	VkDevice device,
	VkBuffer vertexBuffer,
	VkDeviceMemory vertexBufferMemory,
	const VkVertexInputBindingDescription& vertexBindingDescription,
	const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions
)
:	VertexBuffer(bufferSize)
,	m_device(device)
,	m_vertexBuffer(vertexBuffer)
,	m_vertexBufferMemory(vertexBufferMemory)
,	m_vertexBindingDescription(vertexBindingDescription)
,	m_vertexAttributeDescriptions(vertexAttributeDescriptions)
{
}

void VertexBufferVk::destroy()
{
}

void* VertexBufferVk::lock()
{
	void* ptr = 0;
	return (vkMapMemory(m_device, m_vertexBufferMemory, 0, VK_WHOLE_SIZE, 0, &ptr) == VK_SUCCESS) ? ptr : 0;
}

void* VertexBufferVk::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	return 0;
}

void VertexBufferVk::unlock()
{
	vkUnmapMemory(m_device, m_vertexBufferMemory);
	vkBindBufferMemory(m_device, m_vertexBuffer, m_vertexBufferMemory, 0);
}

	}
}
