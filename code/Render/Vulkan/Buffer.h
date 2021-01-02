#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
    namespace render
    {

class Context;

/*!
 * \ingroup Vulkan
 */
class Buffer : public Object
{
    T_RTTI_CLASS;

public:
    Buffer() = delete;

    Buffer(const Buffer&) = delete;

    Buffer(Buffer&&) = default;

    explicit Buffer(Context* context);

    virtual ~Buffer();

	bool create(uint32_t bufferSize, uint32_t usageBits, bool cpuAccess, bool gpuAccess);

	void destroy();

    void* lock();

    void unlock();

    operator VkBuffer () const { return m_buffer; }

private:
    Ref< Context > m_context;
	VmaAllocation m_allocation = 0;
	VkBuffer m_buffer = 0;
    void* m_locked = nullptr;
};
        
    }
}
