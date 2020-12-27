#pragma once

#include <tuple>
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Color4f.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{

class Semaphore;

	namespace render
	{

class CommandBuffer;
class Context;
class RenderTargetDepthVk;
class RenderTargetVk;
struct RenderTargetSetCreateDesc;

/*!
 * \ingroup Vulkan
 */
class RenderTargetSetVk : public IRenderTargetSet
{
	T_RTTI_CLASS;

public:
	explicit RenderTargetSetVk(Context* context);

	virtual ~RenderTargetSetVk();

	bool createPrimary(int32_t width, int32_t height, VkFormat colorFormat, VkImage colorImage, VkFormat depthFormat, VkImage depthImage, const wchar_t* const tag);

	bool create(const RenderTargetSetCreateDesc& setDesc, IRenderTargetSet* sharedDepthStencil, const wchar_t* const tag);

	virtual void destroy() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual ISimpleTexture* getColorTexture(int32_t index) const override final;

	virtual ISimpleTexture* getDepthTexture() const override final;

	virtual bool isContentValid() const override final;

	virtual bool read(int32_t index, void* buffer) const override final;

	virtual void setDebugName(const wchar_t* name) override final;

	bool prepareAsTarget(
		CommandBuffer* commandBuffer,
		int32_t colorIndex,
		const Clear& clear,
		uint32_t load,
		uint32_t store,
		RenderTargetDepthVk* primaryDepthTarget,
		uint32_t& outId,
		VkRenderPass& outRenderPass,
		VkFramebuffer& outFrameBuffer
	);

	bool prepareAsTexture(
		CommandBuffer* commandBuffer,
		int32_t colorIndex
	);

	uint32_t getColorTargetCount() const { return (uint32_t)m_colorTargets.size(); }

	RenderTargetVk* getColorTargetVk(int32_t index) const { return m_colorTargets[index]; }

	RenderTargetDepthVk* getDepthTargetVk() const { return m_depthTarget; }
	
	bool usingPrimaryDepthStencil() const { return m_setDesc.usingPrimaryDepthStencil; }

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

	Ref< Context > m_context;
	RenderTargetSetCreateDesc m_setDesc;
	RefArray< RenderTargetVk > m_colorTargets;
	Ref< RenderTargetDepthVk > m_depthTarget;
	SmallMap< render_pass_key_t, RenderPass > m_renderPasses;
	bool m_depthTargetShared;
};

	}
}
