#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/UniformBufferPoolVk.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.UniformBufferPoolVk", UniformBufferPoolVk, Object)

UniformBufferPoolVk::UniformBufferPoolVk(VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
:	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
{
}

bool UniformBufferPoolVk::acquire(uint32_t size, VkBuffer& outBuffer, VkDeviceMemory& outDeviceMemory)
{
	if (outBuffer != 0)
	{
		T_FATAL_ASSERT(outDeviceMemory != 0);

		BufferChain& bc = m_released.push_back();
		bc.size = size;
		bc.buffer = outBuffer;
		bc.deviceMemory = outDeviceMemory;

		outBuffer = 0;
		outDeviceMemory = 0;
	}

	auto it = std::find_if(m_free.begin(), m_free.end(), [=](const BufferChain& bc) { return bc.size == size; });
	if (it != m_free.end())
	{
		outBuffer = it->buffer;
		outDeviceMemory = it->deviceMemory;
		m_free.erase(it);
	}
	else
	{
		VkBufferCreateInfo bci = {};
		bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bci.pNext = nullptr;
		bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bci.size = size;
		bci.queueFamilyIndexCount = 0;
		bci.pQueueFamilyIndices = nullptr;
		bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bci.flags = 0;
		if (vkCreateBuffer(m_logicalDevice, &bci, nullptr, &outBuffer) != VK_SUCCESS)
			return false;

		VkMemoryRequirements memoryRequirements = {};
		vkGetBufferMemoryRequirements(m_logicalDevice, outBuffer, &memoryRequirements);

		VkMemoryAllocateInfo bai = {};
		bai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		bai.allocationSize = memoryRequirements.size;
		bai.memoryTypeIndex = getMemoryTypeIndex(m_physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryRequirements);
		if (vkAllocateMemory(m_logicalDevice, &bai, nullptr, &outDeviceMemory) != VK_SUCCESS)
			return false;

		vkBindBufferMemory(
			m_logicalDevice,
			outBuffer,
			outDeviceMemory,
			0
		);
	}

	return true;
}

void UniformBufferPoolVk::collect()
{
	if (!m_released.empty())
	{
		m_free.insert(m_free.end(), m_released.begin(), m_released.end());
		m_released.resize(0);
	}
}

	}
}