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

#include <tuple>
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
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
	RenderTargetSetVk(VkPhysicalDevice physicalDevice, VkDevice device);

	virtual ~RenderTargetSetVk();

	bool createPrimary(int32_t width, int32_t height, VkFormat colorFormat, VkImage colorImage, VkFormat depthFormat, VkImage depthImage);

	bool create(const RenderTargetSetCreateDesc& setDesc);

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
		VkCommandBuffer commandBuffer,
		int32_t colorIndex,
		uint32_t clearMask,
		const Color4f* colors,
		float depth,
		int32_t stencil,
		RenderTargetDepthVk* primaryDepthTarget,

		// Out
		uint32_t& outId,
		VkRenderPass& outRenderPass
	);

	bool prepareAsTexture(VkCommandBuffer commandBuffer);

	uint32_t getColorTargetCount() const { return (uint32_t)m_colorTargets.size(); }

	RenderTargetVk* getColorTargetVk(int32_t index) const { return m_colorTargets[index]; }

	RenderTargetDepthVk* getDepthTargetVk() const { return m_depthTarget; }

private:
	struct RenderPass
	{
		uint32_t id;
		VkRenderPass renderPass;
		VkFramebuffer frameBuffer;

		RenderPass()
		:	renderPass(nullptr)
		,	frameBuffer(nullptr)
		{
		}
	};

	typedef std::tuple< int32_t, uint32_t > render_pass_key_t;

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	RenderTargetSetCreateDesc m_setDesc;
	RefArray< RenderTargetVk > m_colorTargets;
	Ref< RenderTargetDepthVk > m_depthTarget;
	SmallMap< render_pass_key_t, RenderPass > m_renderPasses;
};

	}
}

