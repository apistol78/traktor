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
	bool create(VmaAllocator allocator, uint32_t bufferSize, uint32_t usageBits, bool cpuAccess, bool gpuAccess);

	void destroy();

    void* lock();

    void unlock();

    operator VkBuffer () const { return m_buffer; }

private:
	VmaAllocator m_allocator = 0;
	VmaAllocation m_allocation = 0;
	VkBuffer m_buffer = 0;
    bool m_locked = false;
};
        
    }
}
