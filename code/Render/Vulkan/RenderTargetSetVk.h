#pragma once

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

#include "Core/RefArray.h"
#include "Core/Math/Color4f.h"
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

	virtual void destroy() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual ISimpleTexture* getColorTexture(int32_t index) const override final;

	virtual ISimpleTexture* getDepthTexture() const override final;

	virtual void swap(int32_t index1, int32_t index2) override final;

	virtual void discard() override final;

	virtual bool isContentValid() const override final;

	virtual bool read(int32_t index, void* buffer) const override final;

	bool prepareAsTarget(
		VkDevice device,
		VkCommandBuffer commandBuffer,
		int32_t colorIndex,
		uint32_t clearMask,
		const Color4f* colors,
		float depth,
		int32_t stencil,
		RenderTargetDepthVk* primaryDepthTarget
	);

	bool prepareAsTexture(VkCommandBuffer commandBuffer);

	RenderTargetVk* getColorTargetVk(int32_t index) const { return m_colorTargets[index]; }

	RenderTargetDepthVk* getDepthTargetVk() const { return m_depthTarget; }

	VkRenderPass getVkRenderPass() const { return m_renderPass; }

	VkFramebuffer getVkFramebuffer() const { return m_frameBuffer; }

	uint32_t getId() const { return m_id; }

private:
	RenderTargetSetCreateDesc m_setDesc;
	RefArray< RenderTargetVk > m_colorTargets;
	Ref< RenderTargetDepthVk > m_depthTarget;
	VkRenderPass m_renderPass;
	VkFramebuffer m_frameBuffer;
	int32_t m_lastColorIndex;
	uint32_t m_lastClearMask;
	uint32_t m_id;
};

	}
}

