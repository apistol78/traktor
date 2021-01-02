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
class Image : public Object
{
    T_RTTI_CLASS;

public:
    Image() = delete;

    Image(const Image&) = delete;

    Image(Image&&) = default;

    explicit Image(Context* context);

    virtual ~Image();

	bool createSimple(
        uint32_t width,
        uint32_t height,
        uint32_t mipLevels,
        VkFormat format,
        uint32_t usageBits
    );

	bool createCube(
        uint32_t width,
        uint32_t height,
        uint32_t mipLevels,
        VkFormat format,
        uint32_t usageBits
    );

	void destroy();

    void* lock();

    void unlock();

    VkImage getVkImage() const { return m_image; }

    VkImageView getVkImageView() const { return m_imageView; }

private:
    Ref< Context > m_context;
	VmaAllocation m_allocation = 0;
	VkImage m_image = 0;
    VkImageView m_imageView = 0;
    void* m_locked = nullptr;
};
        
    }
}
