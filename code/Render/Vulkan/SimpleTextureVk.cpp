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
	VmaAllocator allocator,
	VkCommandPool setupCommandPool,
	VkQueue setupQueue
)
:	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_allocator(allocator)
,	m_setupCommandPool(setupCommandPool)
,	m_setupQueue(setupQueue)
,	m_stagingBufferAllocation(0)
,	m_stagingBuffer(0)
,	m_textureAllocation(0)
,	m_textureImage(0)
,	m_textureView(0)
{
}

SimpleTextureVk::~SimpleTextureVk()
{
	destroy();
}

bool SimpleTextureVk::create(
	const SimpleTextureCreateDesc& desc,
	const wchar_t* const tag
)
{
	const VkFormat* vkTextureFormats = desc.sRGB ? c_vkTextureFormats_sRGB : c_vkTextureFormats;
	if (vkTextureFormats[desc.format] == VK_FORMAT_UNDEFINED)
	{
		log::error << L"Failed to create VK simple texture; unknown format " << int32_t(desc.format) << L", (" << (desc.sRGB ? L"sRGB" : L"linear") << L")." << Endl;
		return false;
	}

	uint32_t imageSize = getTextureSize(desc.format, desc.width, desc.height, desc.mipCount);

	// Create staging buffer.
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	if (vmaCreateBuffer(m_allocator, &bufferInfo, &aci, &m_stagingBuffer, &m_stagingBufferAllocation, nullptr) != VK_SUCCESS)
		return false;	

	// Copy data into staging buffer.
	uint8_t* data = nullptr;
	if (vmaMapMemory(m_allocator, m_stagingBufferAllocation, (void**)&data) != VK_SUCCESS)
		return false;
	for (int32_t mip = 0; mip < desc.mipCount; ++mip)
	{
		uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, mip);
		std::memcpy(data, desc.initialData[mip].data, mipSize);
		data += mipSize;
	}
	vmaUnmapMemory(m_allocator, m_stagingBufferAllocation);

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
	if (vmaCreateImage(m_allocator, &ici, &aci, &m_textureImage, &m_textureAllocation, nullptr) != VK_SUCCESS)
	{
		log::error << L"Failed to create VK simple texture; unable to allocate image memory." << Endl;
		return false;			
	}

#if !defined(__ANDROID__) && !defined(__APPLE__)
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
		m_setupCommandPool,
		m_setupQueue,
		m_textureImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		0,
		desc.mipCount,
		0,
		1,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	// Copy staging buffer into texture.
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(m_logicalDevice, m_setupCommandPool);

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
			m_stagingBuffer,
			m_textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		offset += mipSize;
	}

	endSingleTimeCommands(m_logicalDevice, m_setupCommandPool, commandBuffer, m_setupQueue);

	// Change layout of texture to optimal sampling.
	changeImageLayout(
		m_logicalDevice,
		m_setupCommandPool,
		m_setupQueue,
		m_textureImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		0,
		desc.mipCount,
		0,
		1,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	// Free staging buffer if immutable.
	if (desc.immutable)
	{
		vkDestroyBuffer(m_logicalDevice, m_stagingBuffer, 0);
		m_stagingBuffer = 0;

		vmaFreeMemory(m_allocator, m_stagingBufferAllocation);
		m_stagingBufferAllocation = 0;
	}

	m_desc = desc;
	return true;
}

void SimpleTextureVk::destroy()
{
	if (m_stagingBufferAllocation != 0)
	{
		vmaFreeMemory(m_allocator, m_stagingBufferAllocation);
		m_stagingBufferAllocation = 0;
	}

	if (m_stagingBuffer != 0)
	{
		vkDestroyBuffer(m_logicalDevice, m_stagingBuffer, 0);
		m_stagingBuffer = 0;
	}

	if (m_textureAllocation != 0)
	{
		vmaFreeMemory(m_allocator, m_textureAllocation);
		m_textureAllocation = 0;
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
	return m_desc.mipCount;
}

int32_t SimpleTextureVk::getWidth() const
{
	return m_desc.width;
}

int32_t SimpleTextureVk::getHeight() const
{
	return m_desc.height;
}

bool SimpleTextureVk::lock(int32_t level, Lock& lock)
{
	if (m_stagingBufferAllocation != 0)
	{
		if (vmaMapMemory(m_allocator, m_stagingBufferAllocation, (void**)&lock.bits) != VK_SUCCESS)
			return false;

		lock.pitch = getTextureRowPitch(m_desc.format, m_desc.width, level);
		return true;
	}
	else
		return false;
}

void SimpleTextureVk::unlock(int32_t level)
{
	vmaUnmapMemory(m_allocator, m_stagingBufferAllocation);	

	// Change layout of texture to be able to copy staging buffer into texture.
	changeImageLayout(
		m_logicalDevice,
		m_setupCommandPool,
		m_setupQueue,
		m_textureImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		level,
		1,
		0,
		1,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	// Copy staging buffer into texture.
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(m_logicalDevice, m_setupCommandPool);

	uint32_t mipWidth = getTextureMipSize(m_desc.width, level);
	uint32_t mipHeight = getTextureMipSize(m_desc.height, level);
	uint32_t mipSize = getTextureMipPitch(m_desc.format, m_desc.width, m_desc.height, level);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = level;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { mipWidth, mipHeight, 1 };

	vkCmdCopyBufferToImage(
		commandBuffer,
		m_stagingBuffer,
		m_textureImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	endSingleTimeCommands(m_logicalDevice, m_setupCommandPool, commandBuffer, m_setupQueue);

	// Change layout of texture to optimal sampling.
	changeImageLayout(
		m_logicalDevice,
		m_setupCommandPool,
		m_setupQueue,
		m_textureImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		level,
		1,
		0,
		1,
		VK_IMAGE_ASPECT_COLOR_BIT
	);
}

void* SimpleTextureVk::getInternalHandle()
{
	return nullptr;
}

	}
}
