/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderTargetDepthVk_H
#define traktor_render_RenderTargetDepthVk_H

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#endif

#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

struct RenderTargetSetCreateDesc;

/*!
 * \ingroup Vulkan
 */
class RenderTargetDepthVk : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	RenderTargetDepthVk();

	virtual ~RenderTargetDepthVk();

	bool createPrimary(VkPhysicalDevice physicalDevice, VkDevice device, int32_t width, int32_t height, VkFormat format, VkImage image);

	bool create(VkPhysicalDevice physicalDevice, VkDevice device, const RenderTargetSetCreateDesc& setDesc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL;

	VkFormat getVkFormat() const { return m_format; }

	VkImage getVkImage() const { return m_image; }

	VkImageView getVkImageView() const { return m_imageView; }

private:
	VkFormat m_format;
	VkImage m_image;
	VkImageView m_imageView;
	int32_t m_width;
	int32_t m_height;
};

	}
}

#endif	// traktor_render_RenderTargetDepthVk_H
