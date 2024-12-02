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

namespace traktor::render
{

class CommandBuffer;
class Context;
class Image;
struct RenderTargetSetCreateDesc;

/*!
 * \ingroup Render
 */
class RenderTargetDepthVk : public ITexture
{
	T_RTTI_CLASS;

public:
	RenderTargetDepthVk() = delete;

	explicit RenderTargetDepthVk(Context* context);

	virtual ~RenderTargetDepthVk();

	bool createPrimary(
		int32_t width,
		int32_t height,
		uint32_t multiSample,
		VkFormat format,
		const wchar_t* const tag
	);

	bool create(const RenderTargetSetCreateDesc& setDesc, const wchar_t* const tag);

	virtual void destroy() override final;

	virtual Size getSize() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	virtual ITexture* resolve() override final;

	VkFormat getVkFormat() const { return m_format; }

	Image* getImage() const { return m_image; }

private:
	friend class RenderTargetSetVk;
	
	Context* m_context = nullptr;
	VkFormat m_format = VK_FORMAT_UNDEFINED;
	Ref< Image > m_image;
	bool m_haveStencil = false;
	int32_t m_width = 0;
	int32_t m_height = 0;

	void prepareAsTarget(CommandBuffer* commandBuffer);

	void prepareAsTexture(CommandBuffer* commandBuffer);
};

}
