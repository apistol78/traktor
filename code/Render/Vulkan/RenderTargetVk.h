/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/ITexture.h"
#include "Render/Vulkan/Private/ApiHeader.h"
#include "Render/Vulkan/Private/Image.h"

namespace traktor::render
{

class Context;
class CommandBuffer;
struct RenderTargetSetCreateDesc;
struct RenderTargetCreateDesc;

/*!
 * \ingroup Render
 */
class RenderTargetVk : public ITexture
{
	T_RTTI_CLASS;

public:
	RenderTargetVk() = delete;

	explicit RenderTargetVk(Context* context);

	virtual ~RenderTargetVk();

	/*! Create primary target.
	 *
	 * \param width Width of target.
	 * \param height Height of target.
	 * \param format Pixel format of target.
	 * \param multiSample Number of samples.
	 * \param swapChainImage Swap chain target image.
	 * \param tag Debug tag.
	 * \return True if target successfully created.
	 */
	bool createPrimary(
		int32_t width,
		int32_t height,
		uint32_t multiSample,
		VkFormat format,
		VkImage swapChainImage,
		const wchar_t* const tag
	);

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, const wchar_t* const tag);

	virtual void destroy() override final;

	virtual Size getSize() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	virtual ITexture* resolve() override final;

	void prepareForPresentation(CommandBuffer* commandBuffer);

	VkFormat getVkFormat() const { return m_format; }

	Image* getImageTarget() const { return m_imageTarget; }

	Image* getImageResolved() const { return m_imageResolved; }

private:
	friend class RenderTargetSetVk;

	Context* m_context = nullptr;
	VkFormat m_format = VK_FORMAT_UNDEFINED;
	Ref< Image > m_imageTarget;
	Ref< Image > m_imageResolved;	//!< For MSAA targets; target image is resolved to this image after rendering.
	int32_t m_width = 0;
	int32_t m_height = 0;

	void prepareAsTarget(CommandBuffer* commandBuffer);

	void prepareAsTexture(CommandBuffer* commandBuffer);

	void prepareForReadBack(CommandBuffer* commandBuffer);
};

}
