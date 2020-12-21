#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/Types.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/Context.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetVk", RenderTargetVk, ISimpleTexture)

RenderTargetVk::RenderTargetVk(Context* context)
:	m_context(context)
,	m_format(VK_FORMAT_UNDEFINED)
,	m_image(0)
,	m_allocation(0)
,	m_imageView(0)
,	m_imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
,	m_width(0)
,	m_height(0)
{
}

RenderTargetVk::~RenderTargetVk()
{
	destroy();
}

bool RenderTargetVk::createPrimary(int32_t width, int32_t height, VkFormat format, VkImage image, const wchar_t* const tag)
{
	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = format;
	ivci.components.r = VK_COMPONENT_SWIZZLE_R;
	ivci.components.g = VK_COMPONENT_SWIZZLE_G;
	ivci.components.b = VK_COMPONENT_SWIZZLE_B;
	ivci.components.a = VK_COMPONENT_SWIZZLE_A;
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
 	if (vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_imageView) != VK_SUCCESS)
		return false;

	m_format = format;
	m_image = image;
	m_width = width;
	m_height = height;

	// Set debug name of texture.
	setObjectDebugName(m_context->getLogicalDevice(), tag, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);
	return true;
}

bool RenderTargetVk::create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, const wchar_t* const tag)
{
	VkResult result;

	VkFormat format = determineSupportedTargetFormat(m_context->getPhysicalDevice(), desc.format);
	if (format == VK_FORMAT_UNDEFINED)
		return false;

	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent.width = setDesc.width;
	imageCreateInfo.extent.height = setDesc.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
 	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	if (vmaCreateImage(m_context->getAllocator(), &imageCreateInfo, &aci, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
		return false;	

	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = m_image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = imageCreateInfo.format;
	ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
 	if ((result = vkCreateImageView(m_context->getLogicalDevice(), &ivci, nullptr, &m_imageView)) != VK_SUCCESS)
	{
		log::error << L"RenderTargetVk::create failed; vkCreateImageView returned error " << getHumanResult(result) << L"." << Endl;
		return false;
	}

	m_format = imageCreateInfo.format;
	m_width = setDesc.width;
	m_height = setDesc.height;

	// Set debug name of texture.
	setObjectDebugName(m_context->getLogicalDevice(), tag, (uint64_t)m_image, VK_OBJECT_TYPE_IMAGE);
	return true;
}

void RenderTargetVk::destroy()
{
	if (m_context)
	{
		m_context->addDeferredCleanup([
			imageView = m_imageView,
			allocation = m_allocation,
			image = m_image
		](Context* cx) {
			if (imageView != 0)
				vkDestroyImageView(cx->getLogicalDevice(), imageView, nullptr);

			// Do not destroy image unless we have allocated memory for it;
			// otherwise it's primary targets thus owned by swapchain.
			if (allocation == 0)
				return;

			vmaFreeMemory(cx->getAllocator(), allocation);
			if (image != 0)
				vkDestroyImage(cx->getLogicalDevice(), image, 0);
		});
	}

	m_context = nullptr;
	m_image = 0;
	m_allocation = 0;
	m_imageView = 0;
}

ITexture* RenderTargetVk::resolve()
{
	return this;
}

int32_t RenderTargetVk::getMips() const
{
	return 1;
}

int32_t RenderTargetVk::getWidth() const
{
	return m_width;
}

int32_t RenderTargetVk::getHeight() const
{
	return m_height;
}

bool RenderTargetVk::lock(int32_t level, Lock& lock)
{
	return false;
}

void RenderTargetVk::unlock(int32_t level)
{
}

void* RenderTargetVk::getInternalHandle()
{
	return nullptr;
}

void RenderTargetVk::prepareForPresentation(VkCommandBuffer cmdBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		return;

	VkImageMemoryBarrier imb = {};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.oldLayout = m_imageLayout;
	imb.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imb.image = m_image;

	imb.srcAccessMask = getAccessMask(imb.oldLayout);
	imb.dstAccessMask = getAccessMask(imb.newLayout);

	vkCmdPipelineBarrier(
		cmdBuffer,
		getPipelineStageFlags(imb.oldLayout),
		getPipelineStageFlags(imb.newLayout),
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	m_imageLayout = imb.newLayout;
}

void RenderTargetVk::prepareAsTarget(VkCommandBuffer cmdBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		return;

	VkImageMemoryBarrier imb = {};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.oldLayout = m_imageLayout;
	imb.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.image = m_image;
	imb.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	imb.srcAccessMask = getAccessMask(imb.oldLayout);
	imb.dstAccessMask = getAccessMask(imb.newLayout);

	vkCmdPipelineBarrier(
		cmdBuffer,
		getPipelineStageFlags(imb.oldLayout),
		getPipelineStageFlags(imb.newLayout),
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	m_imageLayout = imb.newLayout;
}

void RenderTargetVk::prepareAsTexture(VkCommandBuffer cmdBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		return;
	
	VkImageMemoryBarrier imb = {};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.oldLayout = m_imageLayout;
	imb.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imb.image = m_image;

	imb.srcAccessMask = getAccessMask(imb.oldLayout);
	imb.dstAccessMask = getAccessMask(imb.newLayout);

	vkCmdPipelineBarrier(
		cmdBuffer,
		getPipelineStageFlags(imb.oldLayout),
		getPipelineStageFlags(imb.newLayout),
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	m_imageLayout = imb.newLayout;
}

void RenderTargetVk::prepareForReadBack(VkCommandBuffer cmdBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		return;

	VkImageMemoryBarrier imb = {};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.oldLayout = m_imageLayout;
	imb.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imb.image = m_image;

	imb.srcAccessMask = getAccessMask(imb.oldLayout);
	imb.dstAccessMask = getAccessMask(imb.newLayout);

	vkCmdPipelineBarrier(
		cmdBuffer,
		getPipelineStageFlags(imb.oldLayout),
		getPipelineStageFlags(imb.newLayout),
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	m_imageLayout = imb.newLayout;
}

	}
}
