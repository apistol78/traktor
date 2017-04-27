/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IndexBufferVk_H
#define traktor_render_IndexBufferVk_H

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#endif

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

	VkBuffer getVkBuffer() const { return m_indexBuffer; }

private:
	VkDevice m_device;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;
};
	
	}
}

#endif	// traktor_render_IndexBufferVk_H
