#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/StructBufferVk.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferVk", StructBufferVk, StructBuffer)

StructBufferVk::StructBufferVk(uint32_t bufferSize, VkDevice device, VkBuffer storageBuffer, VkDeviceMemory storageBufferMemory)
:	StructBuffer(bufferSize)
,	m_device(device)
,	m_storageBuffer(storageBuffer)
,	m_storageBufferMemory(storageBufferMemory)
{
}

void StructBufferVk::destroy()
{
}

void* StructBufferVk::lock()
{
	void* ptr = nullptr;
	return (vkMapMemory(m_device, m_storageBufferMemory, 0, VK_WHOLE_SIZE, 0, &ptr) == VK_SUCCESS) ? ptr : nullptr;
}

void* StructBufferVk::lock(uint32_t structOffset, uint32_t structCount)
{
	return nullptr;
}

void StructBufferVk::unlock()
{
	vkUnmapMemory(m_device, m_storageBufferMemory);
	vkBindBufferMemory(m_device, m_storageBuffer, m_storageBufferMemory, 0);
}

	}
}