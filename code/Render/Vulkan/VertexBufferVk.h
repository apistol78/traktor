#ifndef traktor_render_VertexBufferVk_H
#define traktor_render_VertexBufferVk_H

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan.h>

#include "Core/Containers/AlignedVector.h"
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
	VertexBufferVk(
		uint32_t bufferSize,
		VkDevice device,
		VkBuffer vertexBuffer,
		VkDeviceMemory vertexBufferMemory,
		const VkVertexInputBindingDescription& vertexBindingDescription,
		const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions
	);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) T_OVERRIDE T_FINAL;

	virtual void unlock() T_OVERRIDE T_FINAL;

	VkBuffer getVkBuffer() const { return m_vertexBuffer; }

private:
	VkDevice m_device;
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkVertexInputBindingDescription m_vertexBindingDescription;
	AlignedVector< VkVertexInputAttributeDescription > m_vertexAttributeDescriptions;
};
	
	}
}

#endif	// traktor_render_VertexBufferVk_H
