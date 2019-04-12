#include "Core/Log/Log.h"
#include "Render/Types.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDepthVk", RenderTargetDepthVk, ISimpleTexture)

RenderTargetDepthVk::RenderTargetDepthVk()
:	m_format(VK_FORMAT_UNDEFINED)
,	m_image(nullptr)
,	m_imageView(nullptr)
,	m_imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
,	m_width(0)
,	m_height(0)
{
}

RenderTargetDepthVk::~RenderTargetDepthVk()
{
	destroy();
}

bool RenderTargetDepthVk::createPrimary(VkPhysicalDevice physicalDevice, VkDevice device, int32_t width, int32_t height, VkFormat format, VkImage image)
{
	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = format;
	ivci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // | VK_IMAGE_ASPECT_STENCIL_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;

 	if (vkCreateImageView(device, &ivci, nullptr, &m_imageView) != VK_SUCCESS)
		return false;

	m_format = format;
	m_image = image;
	m_width = width;
	m_height = height;

	return true;
}

bool RenderTargetDepthVk::create(VkPhysicalDevice physicalDevice, VkDevice device, const RenderTargetSetCreateDesc& setDesc)
{
	VkResult result;

	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_D16_UNORM; // VK_FORMAT_D24_UNORM_S8_UINT;
	imageCreateInfo.extent.width = setDesc.width;
	imageCreateInfo.extent.height = setDesc.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
 	if ((result = vkCreateImage(device, &imageCreateInfo, nullptr, &m_image)) != VK_SUCCESS)
	{
		log::error << L"RenderTargetDepthVk::create failed; vkCreateImage returned error " << getHumanResult(result) << L"." << Endl;
		return false;
	}

	VkMemoryRequirements memoryRequirements = {};
	vkGetImageMemoryRequirements(device, m_image, &memoryRequirements);

	VkMemoryAllocateInfo imageAllocateInfo = {};
	imageAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageAllocateInfo.allocationSize = memoryRequirements.size;
	imageAllocateInfo.memoryTypeIndex = getMemoryTypeIndex(physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryRequirements);

	VkDeviceMemory imageMemory = {};
	if ((result = vkAllocateMemory(device, &imageAllocateInfo, nullptr, &imageMemory)) != VK_SUCCESS)
	{
		log::error << L"RenderTargetDepthVk::create failed; vkAllocateMemory returned error " << getHumanResult(result) << L"." << Endl;
		return false;
	}

	if ((result = vkBindImageMemory(device, m_image, imageMemory, 0)) != VK_SUCCESS)
	{
		log::error << L"RenderTargetDepthVk::create failed; vkBindImageMemory returned error " << getHumanResult(result) << L"." << Endl;
		return false;
	}

	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = m_image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = imageCreateInfo.format;
	ivci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // | VK_IMAGE_ASPECT_STENCIL_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
 	if ((result = vkCreateImageView(device, &ivci, nullptr, &m_imageView)) != VK_SUCCESS)
	{
		log::error << L"RenderTargetDepthVk::create failed; vkCreateImageView returned error " << getHumanResult(result) << L"." << Endl;
		return false;
	}

	m_format = imageCreateInfo.format;
	m_width = setDesc.width;
	m_height = setDesc.height;
	return true;
}

void RenderTargetDepthVk::destroy()
{
}

ITexture* RenderTargetDepthVk::resolve()
{
	return this;
}

int32_t RenderTargetDepthVk::getMips() const
{
	return 1;
}

int32_t RenderTargetDepthVk::getWidth() const
{
	return m_width;
}

int32_t RenderTargetDepthVk::getHeight() const
{
	return m_height;
}

bool RenderTargetDepthVk::lock(int32_t level, Lock& lock)
{
	return false;
}

void RenderTargetDepthVk::unlock(int32_t level)
{
}

void* RenderTargetDepthVk::getInternalHandle()
{
	return nullptr;
}

void RenderTargetDepthVk::prepareAsTarget(VkCommandBuffer cmdBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		return;

	VkImageMemoryBarrier layoutTransitionBarrier = {};
	layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutTransitionBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	layoutTransitionBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT; // | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	layoutTransitionBarrier.oldLayout = m_imageLayout;
	layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.image = m_image;
	layoutTransitionBarrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT /* | VK_IMAGE_ASPECT_STENCIL_BIT */, 0, 1, 0, 1 };

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &layoutTransitionBarrier
	);

	m_imageLayout = layoutTransitionBarrier.newLayout;
}

void RenderTargetDepthVk::prepareAsTexture(VkCommandBuffer cmdBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		return;

	VkImageMemoryBarrier layoutTransitionBarrier = {};
	layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutTransitionBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT; // | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	layoutTransitionBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	layoutTransitionBarrier.oldLayout = m_imageLayout;
	layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT /* | VK_IMAGE_ASPECT_STENCIL_BIT*/, 0, 1, 0, 1 };
	layoutTransitionBarrier.image = m_image;

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &layoutTransitionBarrier
	);

	m_imageLayout = layoutTransitionBarrier.newLayout;
}

	}
}
