/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SimpleTextureVk_H
#define traktor_render_SimpleTextureVk_H

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#elif defined(__LINUX__)
#	define VK_USE_PLATFORM_LINUX_KHR
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

struct SimpleTextureCreateDesc;
		
/*!
 * \ingroup Vulkan
 */
class SimpleTextureVk : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTextureVk();

	virtual ~SimpleTextureVk();

	bool create(VkPhysicalDevice physicalDevice, VkDevice device, const SimpleTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;
	
	virtual bool lock(int level, Lock& lock) override final;

	virtual void unlock(int level) override final;

	virtual void* getInternalHandle() override final;

private:
	VkImage m_image;
	int32_t m_width;
	int32_t m_height;
};
		
	}
}

#endif	// traktor_render_SimpleTextureVk_H
