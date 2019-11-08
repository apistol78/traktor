#include "Core/Log/Log.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/UniformBufferPoolVk.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.UniformBufferPoolVk", UniformBufferPoolVk, Object)

UniformBufferPoolVk::UniformBufferPoolVk(VkDevice logicalDevice, VmaAllocator allocator)
:	m_logicalDevice(logicalDevice)
,	m_allocator(allocator)
,	m_counter(0)
{
}

bool UniformBufferPoolVk::acquire(
	uint32_t size,
	VkBuffer& inoutBuffer,
	VmaAllocation& inoutAllocation
)
{
	if (inoutBuffer != 0)
	{
		T_FATAL_ASSERT(inoutAllocation != 0);

		BufferChain& bc = m_released[m_counter].push_back();
		bc.size = size;
		bc.buffer = inoutBuffer;
		bc.allocation = inoutAllocation;

		inoutBuffer = 0;
		inoutAllocation = 0;
	}

	auto it = std::find_if(m_free.begin(), m_free.end(), [=](const BufferChain& bc) { return bc.size == size; });
	if (it != m_free.end())
	{
		inoutBuffer = it->buffer;
		inoutAllocation = it->allocation;
		m_free.erase(it);
	}
	else
	{
		VkBufferCreateInfo bci = {};
		bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bci.size = size;
		bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo aci = {};
		aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		if (vmaCreateBuffer(m_allocator, &bci, &aci, &inoutBuffer, &inoutAllocation, nullptr) != VK_SUCCESS)
			return false;

#if !defined(__ANDROID__) && !defined(__APPLE__)
		// Set debug name of uniform buffer.
		VkDebugUtilsObjectNameInfoEXT ni = {};
		ni.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		ni.objectType = VK_OBJECT_TYPE_BUFFER;
		ni.objectHandle = (uint64_t)inoutBuffer;
		ni.pObjectName = "Uniform buffer";
		vkSetDebugUtilsObjectNameEXT(m_logicalDevice, &ni);
#endif
	}

	return true;
}

void UniformBufferPoolVk::collect()
{
	uint32_t c = (m_counter + 1) % MaxPendingFrames;
	if (!m_released[c].empty())
	{
		m_free.insert(m_free.end(), m_released[c].begin(), m_released[c].end());
		m_released[c].resize(0);
	}
	m_counter = c;
}

	}
}