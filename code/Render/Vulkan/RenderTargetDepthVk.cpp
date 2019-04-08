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
,	m_image(0)
,	m_imageView(0)
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
	ivci.components.r = VK_COMPONENT_SWIZZLE_R;
	ivci.components.g = VK_COMPONENT_SWIZZLE_G;
	ivci.components.b = VK_COMPONENT_SWIZZLE_B;
	ivci.components.a = VK_COMPONENT_SWIZZLE_A;
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
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
	imageCreateInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
	imageCreateInfo.extent.width = setDesc.width;
	imageCreateInfo.extent.height = setDesc.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
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
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
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
	return 0;
}

	}
}
