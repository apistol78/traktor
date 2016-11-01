#ifndef traktor_render_RenderTargetSetVk_H
#define traktor_render_RenderTargetSetVk_H

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#endif

#include "Core/RefArray.h"
#include "Render/RenderTargetSet.h"

namespace traktor
{
	namespace render
	{

class RenderTargetDepthVk;
class RenderTargetVk;
struct RenderTargetSetCreateDesc;

/*!
 * \ingroup Vulkan
 */
class RenderTargetSetVk : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetVk();

	virtual ~RenderTargetSetVk();

	bool createPrimary(VkPhysicalDevice physicalDevice, VkDevice device, int32_t width, int32_t height, VkFormat colorFormat, VkImage colorImage, VkFormat depthFormat, VkImage depthImage);

	bool create(VkPhysicalDevice physicalDevice, VkDevice device, const RenderTargetSetCreateDesc& setDesc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual ISimpleTexture* getColorTexture(int index) const T_OVERRIDE T_FINAL;

	virtual ISimpleTexture* getDepthTexture() const T_OVERRIDE T_FINAL;

	virtual void swap(int index1, int index2) T_OVERRIDE T_FINAL;

	virtual void discard() T_OVERRIDE T_FINAL;

	virtual bool read(int index, void* buffer) const T_OVERRIDE T_FINAL;

	RenderTargetVk* getColorTargetVk(int32_t index) const { return m_colorTargets[index]; }

	RenderTargetDepthVk* getDepthTargetVk() const { return m_depthTarget; }

	VkRenderPass getVkRenderPass() const { return m_renderPass; }

	VkFramebuffer getVkFramebuffer() const { return m_framebuffer; }

private:
	RefArray< RenderTargetVk > m_colorTargets;
	Ref< RenderTargetDepthVk > m_depthTarget;
	int32_t m_width;
	int32_t m_height;
	VkRenderPass m_renderPass;
	VkFramebuffer m_framebuffer;
};

	}
}

#endif	// traktor_render_RenderTargetSetVk_H
