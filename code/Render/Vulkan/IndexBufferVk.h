#ifndef traktor_render_IndexBufferVk_H
#define traktor_render_IndexBufferVk_H

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan.h>

#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class IndexBufferVk : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferVk(IndexType indexType, uint32_t bufferSize, VkDevice device, VkBuffer indexBuffer, VkDeviceMemory indexBufferMemory);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;
	
	virtual void unlock() T_OVERRIDE T_FINAL;

private:
	VkDevice m_device;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;
};
	
	}
}

#endif	// traktor_render_IndexBufferVk_H
