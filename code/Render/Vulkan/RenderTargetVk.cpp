#include "Render/Types.h"
#include "Render/Vulkan/RenderTargetVk.h"
#if defined(_WIN32)
#	include "Render/Vulkan/Win32/ApiLoader.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

VkFormat convertFormat(TextureFormat textureFormat)
{
	switch (textureFormat)
	{
	case TfR8:
		return VK_FORMAT_R8_UNORM;

	case TfR8G8B8A8:
		return VK_FORMAT_R8G8B8A8_UNORM;

	case TfR5G6B5:
		return VK_FORMAT_R5G6B5_UNORM_PACK16;

	case TfR5G5B5A1:
		return VK_FORMAT_R5G5B5A1_UNORM_PACK16;

	case TfR4G4B4A4:
		return VK_FORMAT_R4G4B4A4_UNORM_PACK16;

	case TfR10G10B10A2:
		return VK_FORMAT_R16G16B16A16_UNORM;

	case TfR16G16B16A16F:
		return VK_FORMAT_R16G16B16A16_SFLOAT;

	case TfR32G32B32A32F:
		return VK_FORMAT_R32G32B32A32_SFLOAT;

	case TfR16G16F:
		return VK_FORMAT_R16G16_SFLOAT;

	case TfR32G32F:
		return VK_FORMAT_R32G32_SFLOAT;

	case TfR16F:
		return VK_FORMAT_R16_SFLOAT;

	case TfR32F:
		return VK_FORMAT_R32_SFLOAT;

	case TfR11G11B10F:
		return VK_FORMAT_R16G16B16A16_SFLOAT;

	//TfDXT1 = 30,
	//TfDXT2 = 31,
	//TfDXT3 = 32,
	//TfDXT4 = 33,
	//TfDXT5 = 34,
	//TfPVRTC1 = 40,	// 4bpp, no alpha
	//TfPVRTC2 = 41,	// 2bpp, no alpha
	//TfPVRTC3 = 42,	// 4bpp, alpha
	//TfPVRTC4 = 43,	// 2bpp, alpha
	//TfETC1 = 44

	default:
		return VK_FORMAT_UNDEFINED;
	}
}

uint32_t getMemoryTypeIndex(VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags memoryFlags, const VkMemoryRequirements& memoryRequirements)
{
	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	uint32_t memoryTypeBits = memoryRequirements.memoryTypeBits;
	for (uint32_t i = 0; i < 32; ++i)
	{
		VkMemoryType memoryType = memoryProperties.memoryTypes[i];
		if (memoryTypeBits & 1)
		{
			if ((memoryType.propertyFlags & memoryFlags) == memoryFlags)
				return i;
		}
		memoryTypeBits = memoryTypeBits >> 1;
	}
	return 0; 
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetVk", RenderTargetVk, ISimpleTexture)

RenderTargetVk::RenderTargetVk()
:	m_format(VK_FORMAT_UNDEFINED)
,	m_image(0)
,	m_imageView(0)
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
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
 	if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_imageView) != VK_SUCCESS)
		return false;

	m_format = format;
	m_image = image;
	m_width = width;
	m_height = height;

	return true;
}

bool RenderTargetVk::create(VkPhysicalDevice physicalDevice, VkDevice device, const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = convertFormat(desc.format);
	imageCreateInfo.extent.width = setDesc.width;
	imageCreateInfo.extent.height = setDesc.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
 	if (vkCreateImage(device, &imageCreateInfo, nullptr, &m_image) != VK_SUCCESS)
		return false;

	VkMemoryRequirements memoryRequirements = {};
	vkGetImageMemoryRequirements(device, m_image, &memoryRequirements);
 
	VkMemoryAllocateInfo imageAllocateInfo = {};
	imageAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageAllocateInfo.allocationSize = memoryRequirements.size;
	imageAllocateInfo.memoryTypeIndex = getMemoryTypeIndex(physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryRequirements);
 
	VkDeviceMemory imageMemory = {};
	if (vkAllocateMemory(device, &imageAllocateInfo, nullptr, &imageMemory) != VK_SUCCESS)
		return 0;
 
	if (vkBindImageMemory(device, m_image, imageMemory, 0) != VK_SUCCESS)
		return 0;

	//if (!performImageTranslation(
	//	device,
	//	presentQueue,
	//	setupCmdBuffer,
	//	m_image,
	//	VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	//	VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	//))
	//	return 0;

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = m_image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = imageCreateInfo.format;
	imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
 	if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_imageView) != VK_SUCCESS)
		return false;

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

int RenderTargetVk::getWidth() const
{
	return m_width;
}

int RenderTargetVk::getHeight() const
{
	return m_height;
}

bool RenderTargetVk::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetVk::unlock(int level)
{
}

void* RenderTargetVk::getInternalHandle()
{
	return 0;
}

	}
}
