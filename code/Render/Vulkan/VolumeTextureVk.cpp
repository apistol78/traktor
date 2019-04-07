#include <cstring>
#include "Render/Types.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/UtilitiesVk.h"
#include "Render/Vulkan/VolumeTextureVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureVk", VolumeTextureVk, IVolumeTexture)

VolumeTextureVk::VolumeTextureVk()
:	m_textureImage(nullptr)
,	m_textureView(nullptr)
,	m_width(0)
,	m_height(0)
,	m_depth(0)
{
}

VolumeTextureVk::~VolumeTextureVk()
{
	destroy();
}

bool VolumeTextureVk::create(
	VkPhysicalDevice physicalDevice,
	VkDevice device,
	VkCommandPool commandPool,
	VkQueue queue,
	const VolumeTextureCreateDesc& desc
)
{
	if (desc.immutable)
	{
		const VkFormat* vkTextureFormats = desc.sRGB ? c_vkTextureFormats_sRGB : c_vkTextureFormats;
		if (vkTextureFormats[desc.format] == VK_FORMAT_UNDEFINED)
			return false;

		uint32_t imageSize = getTextureSize(desc.format, desc.width, desc.height, 1) * desc.depth;

		// Create staging buffer.
		VkBuffer stagingBuffer = nullptr;
		VkDeviceMemory stagingBufferMemory = nullptr;

		if (!createBuffer(
			physicalDevice,
			device,
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		))
			return false;

		// Copy data into staging buffer.
		uint8_t* data = nullptr;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, (void**)&data);
		for (int32_t slice = 0; slice < desc.depth; ++slice)
		{
			uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, 0);
			std::memcpy(data, (uint8_t*)desc.initialData[0].data + desc.initialData[0].slicePitch * slice, mipSize);
			data += mipSize;
		}
		vkUnmapMemory(device, stagingBufferMemory);

		// Create texture image.
		VkImageCreateInfo ici = {};
		ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ici.imageType = VK_IMAGE_TYPE_3D;
		ici.extent.width = desc.width;
		ici.extent.height = desc.height;
		ici.extent.depth = desc.depth;
		ici.mipLevels = desc.mipCount;
		ici.arrayLayers = 1;
		ici.format = vkTextureFormats[desc.format];
		ici.tiling = VK_IMAGE_TILING_OPTIMAL;
		ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ici.samples = VK_SAMPLE_COUNT_1_BIT;
		ici.flags = 0;
		
		if (vkCreateImage(device, &ici, nullptr, &m_textureImage) != VK_SUCCESS)
			return false;

		// Calculate memory requirement of texture image.
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(device, m_textureImage, &memoryRequirements);

		// Allocate texture memory.
		VkDeviceMemory textureImageMemory = nullptr;

		VkMemoryAllocateInfo mai = {};
		mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mai.allocationSize = memoryRequirements.size;
		mai.memoryTypeIndex = getMemoryTypeIndex(physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryRequirements);

		if (vkAllocateMemory(device, &mai, nullptr, &textureImageMemory) != VK_SUCCESS)
			return false;

		vkBindImageMemory(device, m_textureImage, textureImageMemory, 0);

		// Create texture view.
		VkImageViewCreateInfo ivci = {};
		ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivci.image = m_textureImage;
		ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
		ivci.format = vkTextureFormats[desc.format];
		ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ivci.subresourceRange.baseMipLevel = 0;
		ivci.subresourceRange.levelCount = 1;
		ivci.subresourceRange.baseArrayLayer = 0;
		ivci.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &ivci, NULL, &m_textureView) != VK_SUCCESS)
			return false;

		// Change layout of texture to be able to copy staging buffer into texture.
		changeImageLayout(
			device,
			commandPool,
			queue,
			m_textureImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);

		// Copy staging buffer into texture.
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

		uint32_t offset = 0;
		for (int32_t slice = 0; slice < desc.depth; ++slice)
		{
			uint32_t mipWidth = getTextureMipSize(desc.width, 0);
			uint32_t mipHeight = getTextureMipSize(desc.height, 0);
			uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, 0);

			VkBufferImageCopy region = {};
			region.bufferOffset = offset;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, slice };
			region.imageExtent = { mipWidth, mipHeight, 1 };

			vkCmdCopyBufferToImage(
				commandBuffer,
				stagingBuffer,
				m_textureImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region
			);

			offset += mipSize;
		}

		endSingleTimeCommands(device, commandPool, commandBuffer, queue);

		// Change layout of texture to optimal sampling.
		changeImageLayout(
			device,
			commandPool,
			queue,
			m_textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);

		// \tbd Free staging buffer.
		//vkDestroyBuffer(device, stagingBuffer, nullptr);
		//vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	m_width = desc.width;
	m_height = desc.height;
	m_depth = desc.depth;
	return true;
}

void VolumeTextureVk::destroy()
{
}

ITexture* VolumeTextureVk::resolve()
{
	return this;
}

int32_t VolumeTextureVk::getMips() const
{
	return 1;
}

int32_t VolumeTextureVk::getWidth() const
{
	return m_width;
}

int32_t VolumeTextureVk::getHeight() const
{
	return m_height;
}

int32_t VolumeTextureVk::getDepth() const
{
	return m_depth;
}

	}
}
