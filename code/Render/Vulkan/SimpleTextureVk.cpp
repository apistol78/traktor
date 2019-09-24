#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/Types.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/SimpleTextureVk.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureVk", SimpleTextureVk, ISimpleTexture)

SimpleTextureVk::SimpleTextureVk(
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	VmaAllocator allocator
)
:	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_allocator(allocator)
,	m_allocation(0)
,	m_textureImage(0)
,	m_textureView(0)
,	m_mips(0)
,	m_width(0)
,	m_height(0)
{
}

SimpleTextureVk::~SimpleTextureVk()
{
	destroy();
}

bool SimpleTextureVk::create(
	VkCommandPool commandPool,
	VkQueue queue,
	const SimpleTextureCreateDesc& desc,
	const wchar_t* const tag
)
{
	if (desc.immutable)
	{
		const VkFormat* vkTextureFormats = desc.sRGB ? c_vkTextureFormats_sRGB : c_vkTextureFormats;
		if (vkTextureFormats[desc.format] == VK_FORMAT_UNDEFINED)
		{
			log::error << L"Failed to create VK simple texture; unknown format " << int32_t(desc.format) << L", (" << (desc.sRGB ? L"sRGB" : L"linear") << L")." << Endl;
			return false;
		}

		uint32_t imageSize = getTextureSize(desc.format, desc.width, desc.height, desc.mipCount);

		// Create staging buffer.
		VkBuffer stagingBuffer = 0;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = imageSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo aci = {};
		aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VmaAllocation stagingBufferAllocation;
		if (vmaCreateBuffer(m_allocator, &bufferInfo, &aci, &stagingBuffer, &stagingBufferAllocation, nullptr) != VK_SUCCESS)
			return false;	

		// Copy data into staging buffer.
		uint8_t* data = nullptr;
		vmaMapMemory(m_allocator, stagingBufferAllocation, (void**)&data);
		for (int32_t mip = 0; mip < desc.mipCount; ++mip)
		{
			uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, mip);
			std::memcpy(data, desc.initialData[mip].data, mipSize);
			data += mipSize;
		}
		vmaUnmapMemory(m_allocator, stagingBufferAllocation);

		// Create texture image.
		VkImageCreateInfo ici = {};
		ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ici.imageType = VK_IMAGE_TYPE_2D;
		ici.extent.width = desc.width;
		ici.extent.height = desc.height;
		ici.extent.depth = 1;
		ici.mipLevels = desc.mipCount;
		ici.arrayLayers = 1;
		ici.format = vkTextureFormats[desc.format];
		ici.tiling = VK_IMAGE_TILING_OPTIMAL;
		ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ici.samples = VK_SAMPLE_COUNT_1_BIT;
		ici.flags = 0;

		// Non-compressed textures allow for shader storage.
		if (desc.format < TfDXT1)
			ici.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

		aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		if (vmaCreateImage(m_allocator, &ici, &aci, &m_textureImage, &m_allocation, nullptr) != VK_SUCCESS)
		{
			log::error << L"Failed to create VK simple texture; unable to allocate image memory." << Endl;
			return false;			
		}

#if !defined(__ANDROID__)
		// Set debug name of texture.
		VkDebugUtilsObjectNameInfoEXT ni = {};
		ni.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		ni.objectType = VK_OBJECT_TYPE_IMAGE;
		ni.objectHandle = (uint64_t)m_textureImage;
		ni.pObjectName = tag ? wstombs(tag).c_str() : "SimpleTextureVk";
		vkSetDebugUtilsObjectNameEXT(m_logicalDevice, &ni);
#endif

		// Create texture view.
		VkImageViewCreateInfo ivci = {};
		ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivci.image = m_textureImage;
		ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ivci.format = vkTextureFormats[desc.format];
		ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ivci.subresourceRange.baseMipLevel = 0;
		ivci.subresourceRange.levelCount = desc.mipCount;
		ivci.subresourceRange.baseArrayLayer = 0;
		ivci.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_logicalDevice, &ivci, nullptr, &m_textureView) != VK_SUCCESS)
		{
			log::error << L"Failed to create VK simple texture; unable to create image view." << Endl;
			return false;
		}

		// Change layout of texture to be able to copy staging buffer into texture.
		changeImageLayout(
			m_logicalDevice,
			commandPool,
			queue,
			m_textureImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0,
			desc.mipCount
		);

		// Copy staging buffer into texture.
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(m_logicalDevice, commandPool);

		uint32_t offset = 0;
		for (int32_t mip = 0; mip < desc.mipCount; ++mip)
		{
			uint32_t mipWidth = getTextureMipSize(desc.width, mip);
			uint32_t mipHeight = getTextureMipSize(desc.height, mip);
			uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, mip);

			VkBufferImageCopy region = {};
			region.bufferOffset = offset;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = mip;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
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

		endSingleTimeCommands(m_logicalDevice, commandPool, commandBuffer, queue);

		// Change layout of texture to optimal sampling.
		changeImageLayout(
			m_logicalDevice,
			commandPool,
			queue,
			m_textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			0,
			desc.mipCount
		);

		// Free staging buffer.
		vkDestroyBuffer(m_logicalDevice, stagingBuffer, 0);
		vmaFreeMemory(m_allocator, stagingBufferAllocation);
	}

	m_mips = desc.mipCount;
	m_width = desc.width;
	m_height = desc.height;
	return true;
}

void SimpleTextureVk::destroy()
{
	if (m_allocation != 0)
	{
		vmaFreeMemory(m_allocator, m_allocation);
		m_allocation = 0;
	}

	if (m_textureImage != 0)
	{
		vkDestroyImage(m_logicalDevice, m_textureImage, 0);
		m_textureImage = 0;
	}
}

ITexture* SimpleTextureVk::resolve()
{
	return this;
}

int32_t SimpleTextureVk::getMips() const
{
	return m_mips;
}

int32_t SimpleTextureVk::getWidth() const
{
	return m_width;
}

int32_t SimpleTextureVk::getHeight() const
{
	return m_height;
}

bool SimpleTextureVk::lock(int32_t level, Lock& lock)
{
	return false;
}

void SimpleTextureVk::unlock(int32_t level)
{
}

void* SimpleTextureVk::getInternalHandle()
{
	return nullptr;
}

	}
}
