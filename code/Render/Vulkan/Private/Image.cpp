/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Config.h"
#include "Core/Log/Log.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Image.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Image", Image, Object)

Image::Image(Context* context)
:	m_context(context)
{
}

Image::~Image()
{
	T_FATAL_ASSERT_M(m_context == nullptr, L"Image not properly destroyed.");
}

bool Image::createSimple(
	uint32_t width,
	uint32_t height,
	uint32_t mipLevels,
	VkFormat format,
	uint32_t usageBits
)
{
	T_FATAL_ASSERT(m_image == 0);

	// Create image.
	VkImageCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType = VK_IMAGE_TYPE_2D;
	ici.extent.width = width;
	ici.extent.height = height;
	ici.extent.depth = 1;
	ici.mipLevels = mipLevels;
	ici.arrayLayers = 1;
	ici.format = format;
	ici.tiling = VK_IMAGE_TILING_OPTIMAL;
	ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ici.usage = usageBits;
	ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ici.samples = VK_SAMPLE_COUNT_1_BIT;
	ici.flags = 0;

	VmaAllocationCreateInfo aci = {};
	aci.flags = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
	aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	if (vmaCreateImage(m_context->getAllocator(), &ici, &aci, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
	{
		log::error << L"Failed to create image; unable to allocate image memory." << Endl;
		return false;
	}
	setObjectDebugName(m_context->getLogicalDevice(), T_FILE_LINE_W, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);

	// Create image view.
	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = m_image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = format;
	ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = mipLevels;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
	if (vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_imageView) != VK_SUCCESS)
	{
		log::error << L"Failed to create image view; unable to create image view." << Endl;
		return false;
	}
	setObjectDebugName(m_context->getLogicalDevice(), T_FILE_LINE_W, (uint64_t)m_imageView, VK_OBJECT_TYPE_IMAGE_VIEW);

	if ((usageBits & VK_IMAGE_USAGE_STORAGE_BIT) != 0)
	{
		m_storageImageViews.resize(mipLevels);
		for (uint32_t mip = 0; mip < mipLevels; ++mip)
		{
			VkImageViewCreateInfo ivci = {};
			ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ivci.image = m_image;
			ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
			ivci.format = format;
			ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ivci.subresourceRange.baseMipLevel = mip;
			ivci.subresourceRange.levelCount = 1;
			ivci.subresourceRange.baseArrayLayer = 0;
			ivci.subresourceRange.layerCount = 1;
			if (vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_storageImageViews[mip]) != VK_SUCCESS)
			{
				log::error << L"Failed to create image view; unable to create image view." << Endl;
				return false;
			}
		}
	}

	m_mipCount = mipLevels;
	m_layerCount = 1;
	m_imageLayouts.resize(m_mipCount * m_layerCount, VK_IMAGE_LAYOUT_UNDEFINED);

	updateSampledResource();
	if ((usageBits & VK_IMAGE_USAGE_STORAGE_BIT) != 0)
		updateStorageResource();

	return true;
}

bool Image::createCube(
	uint32_t width,
	uint32_t height,
	uint32_t mipLevels,
	VkFormat format,
	uint32_t usageBits
)
{
	T_FATAL_ASSERT(m_image == 0);

	// Create image.
	VkImageCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType = VK_IMAGE_TYPE_2D;
	ici.extent.width = width;
	ici.extent.height = height;
	ici.extent.depth = 1;
	ici.mipLevels = mipLevels;
	ici.arrayLayers = 6;
	ici.format = format;
	ici.tiling = VK_IMAGE_TILING_OPTIMAL;
	ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ici.usage = usageBits;
	ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ici.samples = VK_SAMPLE_COUNT_1_BIT;
	ici.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	VmaAllocationCreateInfo aci = {};
	aci.flags = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
	aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	if (vmaCreateImage(m_context->getAllocator(), &ici, &aci, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
	{
		log::error << L"Failed to create image; unable to allocate image memory." << Endl;
		return false;
	}
	setObjectDebugName(m_context->getLogicalDevice(), T_FILE_LINE_W, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);

	// Create image view.
	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = m_image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	ivci.format = format;
	ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = mipLevels;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 6;
	if (vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_imageView) != VK_SUCCESS)
	{
		log::error << L"Failed to create image view; unable to create image view." << Endl;
		return false;
	}
	setObjectDebugName(m_context->getLogicalDevice(), T_FILE_LINE_W, (uint64_t)m_imageView, VK_OBJECT_TYPE_IMAGE_VIEW);

	if ((usageBits & VK_IMAGE_USAGE_STORAGE_BIT) != 0)
	{
		m_storageImageViews.resize(mipLevels);
		for (uint32_t mip = 0; mip < mipLevels; ++mip)
		{
			VkImageViewCreateInfo ivci = {};
			ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ivci.image = m_image;
			ivci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			ivci.format = format;
			ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ivci.subresourceRange.baseMipLevel = mip;
			ivci.subresourceRange.levelCount = 1;
			ivci.subresourceRange.baseArrayLayer = 0;
			ivci.subresourceRange.layerCount = 1;
			if (vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_storageImageViews[mip]) != VK_SUCCESS)
			{
				log::error << L"Failed to create image view; unable to create image view." << Endl;
				return false;
			}
		}
	}

	m_mipCount = mipLevels;
	m_layerCount = 6;
	m_imageLayouts.resize(m_mipCount * m_layerCount, VK_IMAGE_LAYOUT_UNDEFINED);

	updateSampledResource();
	if ((usageBits & VK_IMAGE_USAGE_STORAGE_BIT) != 0)
		updateStorageResource();

	return true;
}

bool Image::createVolume(
	uint32_t width,
	uint32_t height,
	uint32_t depth,
	uint32_t mipLevels,
	VkFormat format,
	uint32_t usageBits
)
{
	T_FATAL_ASSERT(m_image == 0);

	// Create image.
	VkImageCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType = VK_IMAGE_TYPE_3D;
	ici.extent.width = width;
	ici.extent.height = height;
	ici.extent.depth = depth;
	ici.mipLevels = mipLevels;
	ici.arrayLayers = 1;
	ici.format = format;
	ici.tiling = VK_IMAGE_TILING_OPTIMAL;
	ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ici.usage = usageBits;
	ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ici.samples = VK_SAMPLE_COUNT_1_BIT;
	ici.flags = 0;

	VmaAllocationCreateInfo aci = {};
	aci.flags = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
	aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	if (vmaCreateImage(m_context->getAllocator(), &ici, &aci, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
	{
		log::error << L"Failed to create image; unable to allocate image memory." << Endl;
		return false;
	}
	setObjectDebugName(m_context->getLogicalDevice(), T_FILE_LINE_W, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);

	// Create image view.
	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = m_image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
	ivci.format = format;
	ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = mipLevels;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
	if (vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_imageView) != VK_SUCCESS)
	{
		log::error << L"Failed to create image view; unable to create image view." << Endl;
		return false;
	}
	setObjectDebugName(m_context->getLogicalDevice(), T_FILE_LINE_W, (uint64_t)m_imageView, VK_OBJECT_TYPE_IMAGE_VIEW);

	if ((usageBits & VK_IMAGE_USAGE_STORAGE_BIT) != 0)
	{
		m_storageImageViews.resize(mipLevels);
		for (uint32_t mip = 0; mip < mipLevels; ++mip)
		{
			VkImageViewCreateInfo ivci = {};
			ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ivci.image = m_image;
			ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
			ivci.format = format;
			ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ivci.subresourceRange.baseMipLevel = mip;
			ivci.subresourceRange.levelCount = 1;
			ivci.subresourceRange.baseArrayLayer = 0;
			ivci.subresourceRange.layerCount = 1;
			if (vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_storageImageViews[mip]) != VK_SUCCESS)
			{
				log::error << L"Failed to create image view; unable to create image view." << Endl;
				return false;
			}
		}
	}

	m_mipCount = mipLevels;
	m_layerCount = 1;
	m_imageLayouts.resize(m_mipCount * m_layerCount, VK_IMAGE_LAYOUT_UNDEFINED);

	updateSampledResource();
	if ((usageBits & VK_IMAGE_USAGE_STORAGE_BIT) != 0)
		updateStorageResource();

	return true;
}

bool Image::createTarget(
	uint32_t width,
	uint32_t height,
	uint32_t multiSample,
	VkFormat format,
	VkImage swapChainImage
)
{
	T_FATAL_ASSERT(m_image == 0);

	if (swapChainImage == 0)
	{
		// Create image.
		VkImageCreateInfo ici = {};
		ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ici.imageType = VK_IMAGE_TYPE_2D;
		ici.extent.width = width;
		ici.extent.height = height;
		ici.extent.depth = 1;
		ici.mipLevels = 1;
		ici.arrayLayers = 1;
		ici.format = format;
		ici.tiling = VK_IMAGE_TILING_OPTIMAL;
		ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ici.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ici.samples = (multiSample <= 1) ? VK_SAMPLE_COUNT_1_BIT : (VkSampleCountFlagBits)multiSample;
		ici.flags = 0;
	
		VmaAllocationCreateInfo aci = {};
		aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		if (vmaCreateImage(m_context->getAllocator(), &ici, &aci, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
		{
			log::error << L"Failed to create image; unable to allocate image memory." << Endl;
			return false;
		}
		setObjectDebugName(m_context->getLogicalDevice(), T_FILE_LINE_W, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);
	}
	else
	{
		m_image = swapChainImage;
	}

	// Create image view.
	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = m_image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = format;
	ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
	if (vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_imageView) != VK_SUCCESS)
	{
		log::error << L"Failed to create image view; unable to create image view." << Endl;
		return false;
	}
	setObjectDebugName(m_context->getLogicalDevice(), T_FILE_LINE_W, (uint64_t)m_imageView, VK_OBJECT_TYPE_IMAGE_VIEW);

	m_storageImageViews.push_back(m_imageView);

	m_mipCount = 1;
	m_layerCount = 1;
	m_imageLayouts.resize(m_mipCount * m_layerCount, VK_IMAGE_LAYOUT_UNDEFINED);

	if (swapChainImage == 0)
	{
		updateSampledResource();
		updateStorageResource();
	}

	return true;
}

bool Image::createDepthTarget(
	uint32_t width,
	uint32_t height,
	uint32_t multiSample,
	VkFormat format,
	bool usedAsTexture,
	bool usedAsStorage
)
{
	T_FATAL_ASSERT(m_image == 0);

	bool useStencil = false;
	if (format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
		useStencil = true;

	// Create image.
	VkImageCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType = VK_IMAGE_TYPE_2D;
	ici.extent.width = width;
	ici.extent.height = height;
	ici.extent.depth = 1;
	ici.mipLevels = 1;
	ici.arrayLayers = 1;
	ici.format = format;
	ici.tiling = VK_IMAGE_TILING_OPTIMAL;
	ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ici.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ici.samples = (multiSample <= 1) ? VK_SAMPLE_COUNT_1_BIT : (VkSampleCountFlagBits)multiSample;
	ici.flags = 0;

	if (usedAsTexture)
		ici.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if (usedAsStorage)
		ici.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	if (vmaCreateImage(m_context->getAllocator(), &ici, &aci, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
	{
		log::error << L"Failed to create image; unable to allocate image memory." << Endl;
		return false;
	}
	setObjectDebugName(m_context->getLogicalDevice(), T_FILE_LINE_W, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);

	// Create image view.
	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = m_image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = format;
	ivci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	ivci.subresourceRange.aspectMask = useStencil ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_DEPTH_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
	if (vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_imageView) != VK_SUCCESS)
	{
		log::error << L"Failed to create image view; unable to create image view." << Endl;
		return false;
	}
	setObjectDebugName(m_context->getLogicalDevice(), T_FILE_LINE_W, (uint64_t)m_imageView, VK_OBJECT_TYPE_IMAGE_VIEW);

	m_storageImageViews.push_back(m_imageView);

	m_mipCount = 1;
	m_layerCount = 1;
	m_imageLayouts.resize(m_mipCount * m_layerCount, VK_IMAGE_LAYOUT_UNDEFINED);

	if (usedAsTexture)
		updateSampledResource();

	if (usedAsStorage)
		updateStorageResource();

	return true;
}

void Image::destroy()
{
	T_FATAL_ASSERT(m_locked == nullptr);

	const uint32_t span = (uint32_t)m_storageImageViews.size();

	if (m_allocation != 0)
	{
		m_context->addDeferredCleanup([
			image = m_image,
			allocation = m_allocation
		](Context* cx) {
			vmaDestroyImage(cx->getAllocator(), image, allocation);
		});
	}

	if (m_imageView != 0)
	{
		m_context->addDeferredCleanup([
			imageView = m_imageView
		](Context* cx) {
			vkDestroyImageView(cx->getLogicalDevice(), imageView, nullptr);
		});
	}

	if (!m_storageImageViews.empty())
	{
		for (auto storageImageView : m_storageImageViews)
		{
			if (storageImageView == m_imageView)
				continue;

			m_context->addDeferredCleanup([
				imageView = storageImageView
			](Context* cx) {
				vkDestroyImageView(cx->getLogicalDevice(), imageView, nullptr);
			});
		}
	}

	if (m_sampledResourceIndex != ~0U)
	{
		m_context->freeSampledResourceIndex(m_sampledResourceIndex);
		m_sampledResourceIndex = ~0U;
	}

	if (m_storageResourceIndex != ~0U)
	{
		m_context->freeStorageResourceIndex(m_storageResourceIndex, span);
		m_storageResourceIndex = ~0U;
	}

	m_allocation = 0;
	m_image = 0;
	m_imageView = 0;
	m_storageImageViews.clear();
	m_context = nullptr;
}

void* Image::lock()
{
	if (m_locked)
		return m_locked;

	if (vmaMapMemory(m_context->getAllocator(), m_allocation, &m_locked) != VK_SUCCESS)
		m_locked = nullptr;

	return m_locked;
}

void Image::unlock()
{
	if (m_locked)
	{
		vmaUnmapMemory(m_context->getAllocator(), m_allocation);
		m_locked = nullptr;
	}
}

bool Image::changeLayout(
	CommandBuffer* commandBuffer,
	VkImageLayout newLayout,
	VkImageAspectFlags aspectMask,
	uint32_t mipLevel,
	uint32_t mipCount,
	uint32_t layerLevel,
	uint32_t layerCount
)
{
	T_ASSERT(mipLevel + mipCount <= m_mipCount);
	T_ASSERT(layerLevel + layerCount <= m_layerCount);

	auto imageLayout = m_imageLayouts[layerLevel * m_mipCount + mipLevel];
	if (imageLayout == newLayout)
		return true;

	if (imageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImageMemoryBarrier imb = {};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.oldLayout = imageLayout;
	imb.newLayout = newLayout;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.subresourceRange.aspectMask = aspectMask;
	imb.subresourceRange.baseMipLevel = mipLevel;
	imb.subresourceRange.levelCount = mipCount;
	imb.subresourceRange.baseArrayLayer = layerLevel;
	imb.subresourceRange.layerCount = layerCount;
	imb.image = m_image;
	imb.srcAccessMask = getAccessMask(imb.oldLayout);
	imb.dstAccessMask = getAccessMask(imb.newLayout);

	vkCmdPipelineBarrier(
		*commandBuffer,
		getPipelineStageFlags(imb.oldLayout),
		getPipelineStageFlags(imb.newLayout),
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	for (uint32_t layer = 0; layer < layerCount; ++layer)
	{
		for (uint32_t mip = 0; mip < mipCount; ++mip)
		{
			m_imageLayouts[(layerLevel + layer) * m_mipCount + (mipLevel + mip)] = imb.newLayout;
		}
	}

	return true;
}

bool Image::updateSampledResource()
{
	if (m_sampledResourceIndex == ~0U)
	{
		m_sampledResourceIndex = m_context->allocateSampledResourceIndex();
		if (m_sampledResourceIndex == ~0U)
		{
			log::error << L"Unable to allocate bindless resource index." << Endl;
			return false;
		}
	}

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.sampler = 0;
	imageInfo.imageView = m_imageView;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstSet = m_context->getBindlessTexturesDescriptorSet();
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	write.pImageInfo = &imageInfo;
	write.dstArrayElement = m_sampledResourceIndex;
	write.dstBinding = Context::BindlessTexturesBinding;

	vkUpdateDescriptorSets(
		m_context->getLogicalDevice(),
		1,
		&write,
		0,
		nullptr
	);

	return true;
}

bool Image::updateStorageResource()
{
	const uint32_t span = (uint32_t)m_storageImageViews.size();

	if (m_storageResourceIndex == ~0U)
	{
		m_storageResourceIndex = m_context->allocateStorageResourceIndex(span);
		if (m_storageResourceIndex == ~0U)
		{
			log::error << L"Unable to allocate bindless storage resource index." << Endl;
			return false;
		}
	}

	for (uint32_t i = 0; i < span; ++i)
	{
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.sampler = 0;
		imageInfo.imageView = m_storageImageViews[i];
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = m_context->getBindlessImagesDescriptorSet();
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		write.pImageInfo = &imageInfo;
		write.dstArrayElement = m_storageResourceIndex + i;
		write.dstBinding = Context::BindlessImagesBinding;

		vkUpdateDescriptorSets(
			m_context->getLogicalDevice(),
			1,
			&write,
			0,
			nullptr
		);
	}

	return true;
}

}