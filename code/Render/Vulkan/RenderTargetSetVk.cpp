/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <limits>
#include "Core/Containers/StaticVector.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Atomic.h"
#include "Render/Types.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetVk", RenderTargetSetVk, IRenderTargetSet)

RenderTargetSetVk::RenderTargetSetVk(Context* context, uint32_t& instances)
:	m_context(context)
,	m_instances(instances)
{
	Atomic::increment((int32_t&)m_instances);
}

RenderTargetSetVk::~RenderTargetSetVk()
{
	destroy();
	Atomic::decrement((int32_t&)m_instances);
}

bool RenderTargetSetVk::createPrimary(
	int32_t width,
	int32_t height,
	uint32_t multiSample,
	VkFormat colorFormat,
	VkImage colorImage,
	RenderTargetDepthVk* depthTarget,
	const wchar_t* const tag
)
{
	m_colorTargets.resize(1);
	m_colorTargets[0] = new RenderTargetVk(m_context);
	if (!m_colorTargets[0]->createPrimary(width, height, multiSample, colorFormat, colorImage, tag))
		return false;

	m_depthTarget = depthTarget;
	m_depthTargetShared = true;

	m_setDesc.count = 1;
	m_setDesc.width = width;
	m_setDesc.height = height;
	m_setDesc.multiSample = multiSample;
	m_setDesc.createDepthStencil = true;
	m_setDesc.usingDepthStencilAsTexture = false;
	m_setDesc.usingPrimaryDepthStencil = false;	// We contain primary depth thus do not share.
	m_setDesc.ignoreStencil = false;
	m_setDesc.generateMips = false;
	m_setDesc.targets[0].format = TfR8G8B8A8;
	m_setDesc.targets[0].sRGB = false;

	return true;
}

bool RenderTargetSetVk::create(
	const RenderTargetSetCreateDesc& setDesc,
	IRenderTargetSet* sharedDepthStencil,
	const wchar_t* const tag
)
{
	m_colorTargets.resize(setDesc.count);
	for (int32_t i = 0; i < setDesc.count; ++i)
	{
		m_colorTargets[i] = new RenderTargetVk(m_context);
		if (!m_colorTargets[i]->create(setDesc, setDesc.targets[i], tag))
			return false;
	}

	if (setDesc.createDepthStencil)
	{
		m_depthTarget = new RenderTargetDepthVk(m_context);
		if (!m_depthTarget->create(setDesc, tag))
			return false;
		m_depthTargetShared = false;
	}
	else if (sharedDepthStencil != nullptr)
	{
		m_depthTarget = static_cast< RenderTargetSetVk* >(sharedDepthStencil)->getDepthTargetVk();
		m_depthTargetShared = true;
	}

	m_setDesc = setDesc;
	return true;
}

void RenderTargetSetVk::destroy()
{
	// Destroy color target textures.
	for (auto colorTarget : m_colorTargets)
	{
		if (colorTarget)
			colorTarget->destroy();
	}
	m_colorTargets.resize(0);

	// Only destroy depth target texture if not being shared, else just release reference.
	if (!m_depthTargetShared)
		safeDestroy(m_depthTarget);
	else
		m_depthTarget = nullptr;
}

int32_t RenderTargetSetVk::getWidth() const
{
	return m_setDesc.width;
}

int32_t RenderTargetSetVk::getHeight() const
{
	return m_setDesc.height;
}

ISimpleTexture* RenderTargetSetVk::getColorTexture(int32_t index) const
{
	return index < (int32_t)m_colorTargets.size() ? m_colorTargets[index] : nullptr;
}

ISimpleTexture* RenderTargetSetVk::getDepthTexture() const
{
	return m_depthTarget;
}

bool RenderTargetSetVk::isContentValid() const
{
	return true;
}

bool RenderTargetSetVk::read(int32_t index, void* buffer) const
{
	return false;
}

void RenderTargetSetVk::setDebugName(const wchar_t* name)
{
	for (auto colorTarget : m_colorTargets)
	{
		setObjectDebugName(m_context->getLogicalDevice(), name, (uint64_t)colorTarget->getImageTarget()->getVkImage(), VK_OBJECT_TYPE_IMAGE);
		setObjectDebugName(m_context->getLogicalDevice(), name, (uint64_t)colorTarget->getImageResolved()->getVkImage(), VK_OBJECT_TYPE_IMAGE);
	}
	if (m_depthTarget)
		setObjectDebugName(m_context->getLogicalDevice(), name, (uint64_t)m_depthTarget->getImage()->getVkImage(), VK_OBJECT_TYPE_IMAGE);
}

bool RenderTargetSetVk::prepareAsTarget(
	CommandBuffer* commandBuffer,
	int32_t colorIndex,
	VkRenderPass renderPass,
	RenderTargetDepthVk* primaryDepthTarget,
	VkFramebuffer& outFrameBuffer
)
{
	auto& frameBuffer = m_frameBuffers[renderPass];
	if (frameBuffer == 0)
	{
 		StaticVector< VkImageView, RenderTargetSetCreateDesc::MaxTargets + 1 > fba;
		
		if (colorIndex >= 0)
		{
			fba.push_back(m_colorTargets[colorIndex]->getImageTarget()->getVkImageView());
			if (needResolve())
				fba.push_back(m_colorTargets[colorIndex]->getImageResolved()->getVkImageView());
		}
		else
		{
			for (int32_t i = 0; i < m_setDesc.count; ++i)
			{
				fba.push_back(m_colorTargets[i]->getImageTarget()->getVkImageView());
				if (needResolve())
					fba.push_back(m_colorTargets[i]->getImageResolved()->getVkImageView());
			}
		}

		if (m_depthTarget)
			fba.push_back(m_depthTarget->getImage()->getVkImageView());
		else if (m_setDesc.usingPrimaryDepthStencil)
			fba.push_back(primaryDepthTarget->getImage()->getVkImageView());

		VkFramebufferCreateInfo fbci = {};
		fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbci.renderPass = renderPass;
		fbci.attachmentCount = (uint32_t)fba.size();
		fbci.pAttachments = fba.ptr();
		fbci.width = m_setDesc.width;
		fbci.height = m_setDesc.height;
		fbci.layers = 1;
		if (vkCreateFramebuffer(m_context->getLogicalDevice(), &fbci, nullptr, &frameBuffer) != VK_SUCCESS)
			return false;
	}

	if (colorIndex >= 0)
		m_colorTargets[colorIndex]->prepareAsTarget(commandBuffer);
	else
	{
		for (auto colorTarget : m_colorTargets)
			colorTarget->prepareAsTarget(commandBuffer);
	}

	if (m_depthTarget)
		m_depthTarget->prepareAsTarget(commandBuffer);
	else if (m_setDesc.usingPrimaryDepthStencil)
		primaryDepthTarget->prepareAsTarget(commandBuffer);

	outFrameBuffer = frameBuffer;
	return true;
}

bool RenderTargetSetVk::prepareAsTexture(
	CommandBuffer* commandBuffer,
	int32_t colorIndex
)
{
	if (colorIndex >= 0)
		m_colorTargets[colorIndex]->prepareAsTexture(commandBuffer);
	else
	{
		for (int32_t i = 0; i < m_setDesc.count; ++i)
			m_colorTargets[i]->prepareAsTexture(commandBuffer);
	}
	
	if (m_depthTarget && m_setDesc.usingDepthStencilAsTexture)
		m_depthTarget->prepareAsTexture(commandBuffer);

	return true;
}

}
