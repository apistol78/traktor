/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/Types.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Image.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDepthVk", RenderTargetDepthVk, ITexture)

RenderTargetDepthVk::RenderTargetDepthVk(Context* context)
:	m_context(context)
{
}

RenderTargetDepthVk::~RenderTargetDepthVk()
{
	destroy();
}

bool RenderTargetDepthVk::createPrimary(
	int32_t width,
	int32_t height,
	uint32_t multiSample,
	VkFormat format,
	const wchar_t* const tag
)
{
	m_image = new Image(m_context);
	if (!m_image->createDepthTarget(width, height, multiSample, format, false, false))
		return false;

	m_context->setObjectDebugName(tag, (uint64_t)m_image->getVkImage(), VK_OBJECT_TYPE_IMAGE);

	m_format = format;
	m_haveStencil = true;
	m_width = width;
	m_height = height;
	return true;
}

bool RenderTargetDepthVk::create(const RenderTargetSetCreateDesc& setDesc, const wchar_t* const tag)
{
	// Pick the best depth/stencil format the device actually supports for an
	// optimal-tiled depth/stencil attachment. The preference lists are ordered
	// to land on a D24-equivalent first, with progressively more permissive
	// fallbacks (32-bit float, then 16-bit).
	//
	// Per the Vulkan spec, every implementation must support at least one of
	// the first two entries in each list, so selection cannot fail on a
	// conformant driver — but we still verify to be safe.
	VkFormat format = VK_FORMAT_UNDEFINED;
	if (setDesc.ignoreStencil)
	{
		const VkFormat candidates[] = {
			VK_FORMAT_X8_D24_UNORM_PACK32,	// 24-bit unorm depth, no stencil — closest match to D24.
			VK_FORMAT_D32_SFLOAT,			// Higher-precision fallback, ubiquitous on desktop.
			VK_FORMAT_D16_UNORM				// Mandatory; last-resort fallback for mobile.
		};
		format = determineSupportedDepthTargetFormat(
			m_context->getPhysicalDevice(),
			candidates,
			sizeof_array(candidates),
			setDesc.usingDepthStencilAsTexture
		);
	}
	else
	{
		const VkFormat candidates[] = {
			VK_FORMAT_D24_UNORM_S8_UINT,	// 24-bit depth + 8-bit stencil — direct D24S8 match.
			VK_FORMAT_D32_SFLOAT_S8_UINT,	// Always supported on devices lacking D24S8 (e.g. MoltenVK).
			VK_FORMAT_D16_UNORM_S8_UINT		// Mobile fallback.
		};
		format = determineSupportedDepthTargetFormat(
			m_context->getPhysicalDevice(),
			candidates,
			sizeof_array(candidates),
			setDesc.usingDepthStencilAsTexture
		);
	}

	if (format == VK_FORMAT_UNDEFINED)
	{
		log::error << L"Failed to create depth target; no supported depth"
			<< (setDesc.ignoreStencil ? L"" : L"/stencil")
			<< L" format on this device." << Endl;
		return false;
	}

	m_image = new Image(m_context);
	if (!m_image->createDepthTarget(
		setDesc.width,
		setDesc.height,
		setDesc.multiSample,
		format,
		setDesc.usingDepthStencilAsTexture,
		setDesc.usingDepthStencilAsStorage
	))
		return false;

	m_context->setObjectDebugName(tag, (uint64_t)m_image->getVkImage(), VK_OBJECT_TYPE_IMAGE);

	m_format = format;
	m_haveStencil = !setDesc.ignoreStencil;
	m_width = setDesc.width;
	m_height = setDesc.height;
	return true;
}

void RenderTargetDepthVk::destroy()
{
	safeDestroy(m_image);
	m_context = nullptr;
}

ITexture::Size RenderTargetDepthVk::getSize() const
{
	return { m_width, m_height, 1, 1 };
}

int32_t RenderTargetDepthVk::getBindlessIndex() const
{
	return -1;
}

bool RenderTargetDepthVk::lock(int32_t side, int32_t level, Lock& lock)
{
	return false;
}

void RenderTargetDepthVk::unlock(int32_t side, int32_t level)
{
}

ITexture* RenderTargetDepthVk::resolve()
{
	return this;
}

void RenderTargetDepthVk::prepareAsTarget(CommandBuffer* commandBuffer)
{
	m_image->changeLayout(
		commandBuffer,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		m_haveStencil ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_DEPTH_BIT,
		0,
		1,
		0,
		1
	);
}

void RenderTargetDepthVk::prepareAsTexture(CommandBuffer* commandBuffer)
{
	m_image->changeLayout(
		commandBuffer,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		m_haveStencil ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_DEPTH_BIT,
		0,
		1,
		0,
		1
	);
}

}
