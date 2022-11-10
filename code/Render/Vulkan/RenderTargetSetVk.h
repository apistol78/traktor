/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <tuple>
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Color4f.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
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
	RenderTargetSetVk() = delete;

	explicit RenderTargetSetVk(Context* context, uint32_t& instances);

	virtual ~RenderTargetSetVk();

	bool createPrimary(
		int32_t width,
		int32_t height,
		uint32_t multiSample,
		VkFormat colorFormat,
		VkImage colorImage,
		RenderTargetDepthVk* depthTarget,
		const wchar_t* const tag
	);

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
		VkRenderPass renderPass,
		RenderTargetDepthVk* primaryDepthTarget,
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

	bool needResolve() const { return m_setDesc.multiSample > 1; }

	VkSampleCountFlagBits getVkSampleCount() const { return needResolve() ? (VkSampleCountFlagBits)m_setDesc.multiSample : VK_SAMPLE_COUNT_1_BIT; }

private:
	Context* m_context = nullptr;
	uint32_t& m_instances;
	RenderTargetSetCreateDesc m_setDesc;
	RefArray< RenderTargetVk > m_colorTargets;
	Ref< RenderTargetDepthVk > m_depthTarget;
	SmallMap< VkRenderPass, VkFramebuffer > m_frameBuffers;
	bool m_depthTargetShared = false;
};

}
