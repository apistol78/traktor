#pragma once

#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
    namespace render
    {

/*!
 * \ingroup Vulkan
 */
class Buffer
{
public:
	bool create(VkDevice logicalDevice, VmaAllocator allocator, uint32_t bufferSize, uint32_t usageBits, bool cpuAccess, bool gpuAccess);

	void destroy();

    void* lock();

    void unlock();

    operator VkBuffer () const { return m_buffer; }

private:
    VkDevice m_logicalDevice = 0;
	VmaAllocator m_allocator = 0;
	VmaAllocation m_allocation = 0;
	VkBuffer m_buffer = 0;
    void* m_locked = nullptr;
};
        
    }
}
