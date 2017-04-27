/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Render/Types.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/SimpleTextureVk.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureVk", SimpleTextureVk, ISimpleTexture)

SimpleTextureVk::SimpleTextureVk()
:	m_image(0)
,	m_width(0)
,	m_height(0)
{
}

SimpleTextureVk::~SimpleTextureVk()
{
	destroy();
}

bool SimpleTextureVk::create(VkPhysicalDevice physicalDevice, VkDevice device, const SimpleTextureCreateDesc& desc)
{
	if (desc.immutable)
	{
		VkImage stagingImage;
		VkDeviceMemory stagingImageMemory;

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = desc.width;
		imageInfo.extent.height = desc.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1; // desc.mipCount;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;
		if (vkCreateImage(device, &imageInfo, nullptr, &stagingImage) != VK_SUCCESS)
			return false;

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(device, stagingImage, &memoryRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = getMemoryTypeIndex(physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryRequirements);
		if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingImageMemory) != VK_SUCCESS)
			return false;

		vkBindImageMemory(device, stagingImage, stagingImageMemory, 0);

		uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, 0);

		void* data;
		if (vkMapMemory(device, stagingImageMemory, 0, mipSize, 0, &data) != VK_SUCCESS)
			return false;

		std::memcpy(data, desc.initialData[0].data, mipSize);

		vkUnmapMemory(device, stagingImageMemory);
	}

	return true;
}

void SimpleTextureVk::destroy()
{
}

ITexture* SimpleTextureVk::resolve()
{
	return this;
}

int SimpleTextureVk::getWidth() const
{
	return m_width;
}

int SimpleTextureVk::getHeight() const
{
	return m_height;
}

bool SimpleTextureVk::lock(int level, Lock& lock)
{
	return true;
}

void SimpleTextureVk::unlock(int level)
{
}

void* SimpleTextureVk::getInternalHandle()
{
	return 0;
}

	}
}
