/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Murmur3.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/RenderPassCache.h"

namespace traktor::render
{

bool operator < (const RenderPassCache::Specification& lh, const RenderPassCache::Specification& rh)
{
	return std::memcmp(&lh, &rh, sizeof(RenderPassCache::Specification)) < 0;
}

bool operator > (const RenderPassCache::Specification& lh, const RenderPassCache::Specification& rh)
{
	return std::memcmp(&lh, &rh, sizeof(RenderPassCache::Specification)) > 0;
}

bool operator == (const RenderPassCache::Specification& lh, const RenderPassCache::Specification& rh)
{
	return std::memcmp(&lh, &rh, sizeof(RenderPassCache::Specification)) == 0;
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderPassCache", RenderPassCache, Object)

RenderPassCache::RenderPassCache(VkDevice logicalDevice)
:	m_logicalDevice(logicalDevice)
{
}

bool RenderPassCache::get(
	const Specification& spec,
	VkRenderPass& outRenderPass
)
{
	const uint32_t h = spec.hash();

	auto it = m_renderPasses.find(h);
	if (it != m_renderPasses.end())
	{
		outRenderPass = it->second;
		return true;
	}

	const bool msaaResolve = (bool)(spec.msaaSampleCount > 1);

	StaticVector< VkAttachmentDescription, RenderTargetSetCreateDesc::MaxTargets * 2 + 1 > passAttachments;

	// We do not wish to support loading of MSAA target which implies
	// we might need to store it also.
// #if defined(_DEBUG)
// 	if (msaaResolve)
// 	{
// 		bool cl = ((spec.clear & CfColor) != 0);
// 		bool ld = ((spec.load & TfColor) != 0);
// 		T_FATAL_ASSERT(!(!cl && ld));
// 	}
// #endif

	for (int32_t i = 0; spec.colorTargetFormats[i] != VK_FORMAT_UNDEFINED && i < RenderTargetSetCreateDesc::MaxTargets; ++i)
	{
		VkAttachmentDescription pa = {};
		pa.format = spec.colorTargetFormats[i];
		pa.samples = (VkSampleCountFlagBits)spec.msaaSampleCount;

		if ((spec.clear & CfColor) != 0)
			pa.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		else if ((spec.load & TfColor) != 0)
			pa.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		else
			pa.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			
		if (!msaaResolve && (spec.store & TfColor) != 0)
			pa.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		else
			pa.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		// Not used in color targets.
		pa.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		pa.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				
		if ((spec.clear & CfColor) != 0)
			pa.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;	// If we're clearing let's also assume we don't know about layout.
		else
			pa.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// Must keep last layout since we're loading existing color.
			
		pa.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		passAttachments.push_back(pa);

		// Add resolve targets.
		if (msaaResolve)
		{
			VkAttachmentDescription pa = {};
			pa.format = spec.colorTargetFormats[i];
			pa.samples = VK_SAMPLE_COUNT_1_BIT;
			pa.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			pa.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			pa.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			pa.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			pa.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			pa.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			passAttachments.push_back(pa);
		}
	}

	if (spec.depthTargetFormat != VK_FORMAT_UNDEFINED)
	{
		VkAttachmentDescription pa = {};
		pa.format = spec.depthTargetFormat;
		pa.samples = (VkSampleCountFlagBits)spec.msaaSampleCount;

		if ((spec.clear & CfDepth) != 0)
			pa.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		else if ((spec.load & TfDepth) != 0)
			pa.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		else
			pa.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			
		if ((spec.store & TfDepth) != 0)
			pa.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		else
			pa.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		if ((spec.clear & CfStencil) != 0)
			pa.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		else if ((spec.load & TfDepth) != 0)
			pa.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		else
			pa.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

		if ((spec.store & TfDepth) != 0)
			pa.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		else
			pa.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		pa.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		pa.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		passAttachments.push_back(pa);
	}

	StaticVector< VkAttachmentReference, RenderTargetSetCreateDesc::MaxTargets > colorAttachmentReferences;
	StaticVector< VkAttachmentReference, RenderTargetSetCreateDesc::MaxTargets > resolveAttachmentReferences;
	if (msaaResolve)
	{
		for (int32_t i = 0; spec.colorTargetFormats[i] != VK_FORMAT_UNDEFINED && i < RenderTargetSetCreateDesc::MaxTargets; ++i)
		{
			auto& car = colorAttachmentReferences.push_back();
			car.attachment = i * 2;
			car.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			auto& rar = resolveAttachmentReferences.push_back();
			rar.attachment = i * 2 + 1;
			rar.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}
	else
	{
		for (int32_t i = 0; spec.colorTargetFormats[i] != VK_FORMAT_UNDEFINED && i < RenderTargetSetCreateDesc::MaxTargets; ++i)
		{
			auto& car = colorAttachmentReferences.push_back();
			car.attachment = i;
			car.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}

	// Depth attachment is always last.
	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = (uint32_t)passAttachments.size() - 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();
	subpass.pColorAttachments = colorAttachmentReferences.c_ptr();
	if (!resolveAttachmentReferences.empty())
		subpass.pResolveAttachments = resolveAttachmentReferences.c_ptr();
	if (spec.depthTargetFormat != VK_FORMAT_UNDEFINED)
		subpass.pDepthStencilAttachment = &depthAttachmentReference;

	VkRenderPassCreateInfo rpci = {};
	rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpci.attachmentCount = (uint32_t)passAttachments.size();
	rpci.pAttachments = passAttachments.ptr();
	rpci.subpassCount = 1;
	rpci.pSubpasses = &subpass;

	//VkSubpassDependency dependencies[] = { {}, {} };
	//if (device.FamilyQueues().graphics.index == device.FamilyQueues().present.index)
	//if (msaaResolve)
	//{
	//	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	//	dependencies[0].dstSubpass = 0;
	//	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	//	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	//	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	//	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	//	dependencies[1].srcSubpass = 0;
	//	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	//	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	//	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	//	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	//	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	//	rpci.dependencyCount = 2;
	//	rpci.pDependencies = dependencies;
	//}

	if (vkCreateRenderPass(m_logicalDevice, &rpci, nullptr, &outRenderPass) != VK_SUCCESS)
		return false;

	m_renderPasses.insert(h, outRenderPass);

	log::debug << L"Render pass created (" << (uint32_t)m_renderPasses.size() << L" render passes)." << Endl;
	return true;
}

uint32_t RenderPassCache::Specification::hash() const
{
	Murmur3 ch;
	ch.begin();
	ch.feed(*this);
	ch.end();
	return ch.get();
}

}
