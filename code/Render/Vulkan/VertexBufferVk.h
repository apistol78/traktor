#ifndef traktor_render_VertexBufferVk_H
#define traktor_render_VertexBufferVk_H

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan.h>

#include "Render/VertexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class VertexBufferVk : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	VertexBufferVk(uint32_t bufferSize, VkDevice device, VkBuffer vertexBuffer, VkDeviceMemory vertexBufferMemory);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) T_OVERRIDE T_FINAL;

	virtual void unlock() T_OVERRIDE T_FINAL;

private:
	VkDevice m_device;
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
};
	
	}
}

#endif	// traktor_render_VertexBufferVk_H
