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
	RenderTargetSetVk(
		VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VkCommandPool setupCommandPool,
		VkQueue setupQueue
	);

	virtual ~RenderTargetSetVk();

	bool createPrimary(int32_t width, int32_t height, VkFormat colorFormat, VkImage colorImage, VkFormat depthFormat, VkImage depthImage);

	bool create(const RenderTargetSetCreateDesc& setDesc);

	virtual void destroy() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual ISimpleTexture* getColorTexture(int32_t index) const override final;

	virtual ISimpleTexture* getDepthTexture() const override final;

	virtual void discard() override final;

	virtual bool isContentValid() const override final;

	virtual bool read(int32_t index, void* buffer) const override final;

	bool prepareAsTarget(
		VkCommandBuffer commandBuffer,
		int32_t colorIndex,
		const Clear& clear,
		RenderTargetDepthVk* primaryDepthTarget,

		// Out
		uint32_t& outId,
		VkRenderPass& outRenderPass
	);

	bool prepareAsTexture(
		VkCommandBuffer commandBuffer,
		int32_t colorIndex
	);

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
		:	renderPass(0)
		,	frameBuffer(0)
		{
		}
	};

	typedef std::tuple< int32_t, uint32_t > render_pass_key_t;

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VkCommandPool m_setupCommandPool;
	VkQueue m_setupQueue;

	RenderTargetSetCreateDesc m_setDesc;
	RefArray< RenderTargetVk > m_colorTargets;
	Ref< RenderTargetDepthVk > m_depthTarget;
	
	SmallMap< render_pass_key_t, RenderPass > m_renderPasses;
};

	}
}

