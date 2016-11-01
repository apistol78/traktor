#ifndef traktor_render_SimpleTextureVk_H
#define traktor_render_SimpleTextureVk_H

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

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL;

private:
	VkImage m_image;
	int32_t m_width;
	int32_t m_height;
};
		
	}
}

#endif	// traktor_render_SimpleTextureVk_H
