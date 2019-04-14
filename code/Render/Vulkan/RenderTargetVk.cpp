#include "Core/Log/Log.h"
#include "Render/Types.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetVk", RenderTargetVk, ISimpleTexture)

RenderTargetVk::RenderTargetVk()
:	m_format(VK_FORMAT_UNDEFINED)
,	m_image(nullptr)
,	m_imageView(nullptr)
,	m_imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
,	m_accessFlags(VK_ACCESS_MEMORY_READ_BIT)
,	m_width(0)
,	m_height(0)
{
}

RenderTargetVk::~RenderTargetVk()
{
	destroy();
}

bool RenderTargetVk::createPrimary(VkPhysicalDevice physicalDevice, VkDevice device, int32_t width, int32_t height, VkFormat format, VkImage image)
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

 	if (vkCreateImageView(device, &ivci, nullptr, &m_imageView) != VK_SUCCESS)
		return false;

	m_format = format;
	m_image = image;
	m_width = width;
	m_height = height;

	return true;
}

bool RenderTargetVk::create(VkPhysicalDevice physicalDevice, VkDevice device, const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc)
{
	VkResult result;

	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = c_vkTextureFormats[desc.format];
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
 	if ((result = vkCreateImage(device, &imageCreateInfo, nullptr, &m_image)) != VK_SUCCESS)
	{
		log::error << L"RenderTargetVk::create failed; vkCreateImage returned error " << getHumanResult(result) << L"." << Endl;
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
		log::error << L"RenderTargetVk::create failed; vkAllocateMemory returned error " << getHumanResult(result) << L"." << Endl;
		return false;
	}

	if ((result = vkBindImageMemory(device, m_image, imageMemory, 0)) != VK_SUCCESS)
	{
		log::error << L"RenderTargetVk::create failed; vkBindImageMemory returned error " << getHumanResult(result) << L"." << Endl;
		return false;
	}

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
 	if ((result = vkCreateImageView(device, &ivci, nullptr, &m_imageView)) != VK_SUCCESS)
	{
		log::error << L"RenderTargetVk::create failed; vkCreateImageView returned error " << getHumanResult(result) << L"." << Endl;
		return false;
	}

	m_format = imageCreateInfo.format;
	m_width = setDesc.width;
	m_height = setDesc.height;
	return true;
}

void RenderTargetVk::destroy()
{
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

void RenderTargetVk::prepareAsTarget(VkCommandBuffer cmdBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		return;

	VkImageMemoryBarrier imb = {};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.srcAccessMask = m_accessFlags;
	imb.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imb.oldLayout = m_imageLayout;
	imb.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.image = m_image;
	imb.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	m_imageLayout = imb.newLayout;
	m_accessFlags = imb.dstAccessMask;
}

void RenderTargetVk::prepareForPresentation(VkCommandBuffer cmdBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		return;

	T_ASSERT_M(m_imageLayout != VK_IMAGE_LAYOUT_UNDEFINED, L"RT have not been rendered into yet.");

	VkImageMemoryBarrier imb = {};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.srcAccessMask = m_accessFlags;
	imb.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	imb.oldLayout = m_imageLayout;
	imb.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imb.image = m_image;

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	m_imageLayout = imb.newLayout;
	m_accessFlags = imb.dstAccessMask;
}

void RenderTargetVk::prepareAsTexture(VkCommandBuffer cmdBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		return;
	
	T_ASSERT_M(m_imageLayout != VK_IMAGE_LAYOUT_UNDEFINED, L"RT have not been rendered into yet.");

	VkImageMemoryBarrier imb = {};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.srcAccessMask = m_accessFlags;
	imb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imb.oldLayout = m_imageLayout;
	imb.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imb.image = m_image;

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	m_imageLayout = imb.newLayout;
	m_accessFlags = imb.dstAccessMask;
}

void RenderTargetVk::prepareForReadBack(VkCommandBuffer cmdBuffer)
{
	if (m_imageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		return;

	T_ASSERT_M(m_imageLayout != VK_IMAGE_LAYOUT_UNDEFINED, L"RT have not been rendered into yet.");

	VkImageMemoryBarrier imb = {};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.srcAccessMask = m_accessFlags;
	imb.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	imb.oldLayout = m_imageLayout;
	imb.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imb.image = m_image;

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imb
	);

	m_imageLayout = imb.newLayout;
	m_accessFlags = imb.dstAccessMask;
}

	}
}
