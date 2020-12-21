#include <algorithm>
#include "Core/Log/Log.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/Context.h"
#include "Render/Vulkan/UniformBufferPoolVk.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

int32_t freeIndex(uint32_t size)
{
	int32_t index = (int32_t)(size / 16);
	if (index > 63)
		index = 63;
	return index;
}

		}
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.UniformBufferPoolVk", UniformBufferPoolVk, Object)

UniformBufferPoolVk::UniformBufferPoolVk(Context* context)
:	m_context(context)
,	m_counter(0)
{
}

bool UniformBufferPoolVk::acquire(
	uint32_t size,
	VkBuffer& inoutBuffer,
	VmaAllocation& inoutAllocation,
	void*& inoutMappedPtr
)
{
	if (inoutBuffer != 0)
	{
		T_FATAL_ASSERT(inoutAllocation != 0);

		BufferChain& bc = m_released[m_counter].push_back();
		bc.size = size;
		bc.buffer = inoutBuffer;
		bc.allocation = inoutAllocation;
		bc.mappedPtr = inoutMappedPtr;

		inoutBuffer = 0;
		inoutAllocation = 0;
		inoutMappedPtr = nullptr;
	}

	// Get free list from size.
	auto& free = m_free[freeIndex(size)];

	// Find matching buffer from free list.
	auto it = std::find_if(free.begin(), free.end(), [=](const BufferChain& bc) { return bc.size == size; });
	if (it != free.end())
	{
		inoutBuffer = it->buffer;
		inoutAllocation = it->allocation;
		inoutMappedPtr = it->mappedPtr;
		
		if (it != free.end() - 1)
			*it = free.back();

		free.pop_back();
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
		aci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo ai = {};
		if (vmaCreateBuffer(m_context->getAllocator(), &bci, &aci, &inoutBuffer, &inoutAllocation, &ai) != VK_SUCCESS)
			return false;

		setObjectDebugName(m_context->getLogicalDevice(), L"Uniform buffer", (uint64_t)inoutBuffer, VK_OBJECT_TYPE_BUFFER);

		inoutMappedPtr = ai.pMappedData;
	}

	return true;
}

void UniformBufferPoolVk::collect()
{
	uint32_t c = (m_counter + 1) % MaxPendingFrames;
	if (!m_released[c].empty())
	{
		for (const auto& released : m_released[c])
		{
			auto& free = m_free[freeIndex(released.size)];
			free.push_back(released);
		}
		m_released[c].resize(0);
	}
	m_counter = c;
}

	}
}