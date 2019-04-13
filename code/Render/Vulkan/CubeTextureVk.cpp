#include <cstring>
#include "Render/Types.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/CubeTextureVk.h"
#include "Render/Vulkan/UtilitiesVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubeTextureVk", CubeTextureVk, ICubeTexture)

CubeTextureVk::CubeTextureVk(
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	VkCommandPool setupCommandPool,
	VkQueue setupQueue,
	const CubeTextureCreateDesc& desc
)
:	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_setupCommandPool(setupCommandPool)
,	m_setupQueue(setupQueue)
,	m_desc(desc)
,	m_textureImage(nullptr)
,	m_textureImageMemory(nullptr)
,	m_textureView(nullptr)
,	m_stagingBuffer(nullptr)
,	m_stagingBufferMemory(nullptr)
{
}

CubeTextureVk::~CubeTextureVk()
{
	destroy();
}

bool CubeTextureVk::create()
{
	const VkFormat* vkTextureFormats = m_desc.sRGB ? c_vkTextureFormats_sRGB : c_vkTextureFormats;
	if (vkTextureFormats[m_desc.format] == VK_FORMAT_UNDEFINED)
		return false;

	// Create texture image.
	VkImageCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType = VK_IMAGE_TYPE_2D;
	ici.extent.width = m_desc.side;
	ici.extent.height = m_desc.side;
	ici.extent.depth = 1;
	ici.mipLevels = m_desc.mipCount;
	ici.arrayLayers = 6;
	ici.format = vkTextureFormats[m_desc.format];
	ici.tiling = VK_IMAGE_TILING_OPTIMAL;
	ici.initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ici.samples = VK_SAMPLE_COUNT_1_BIT;
	ici.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	
	if (vkCreateImage(m_logicalDevice, &ici, nullptr, &m_textureImage) != VK_SUCCESS)
		return false;

	// Calculate memory requirement of texture image.
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(m_logicalDevice, m_textureImage, &memoryRequirements);

	// Allocate texture memory.
	VkMemoryAllocateInfo mai = {};
	mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mai.allocationSize = memoryRequirements.size;
	mai.memoryTypeIndex = getMemoryTypeIndex(m_physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryRequirements);

	if (vkAllocateMemory(m_logicalDevice, &mai, nullptr, &m_textureImageMemory) != VK_SUCCESS)
		return false;

	vkBindImageMemory(m_logicalDevice, m_textureImage, m_textureImageMemory, 0);

	// Create texture view.
	VkImageViewCreateInfo ivci = {};
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = m_textureImage;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	ivci.format = vkTextureFormats[m_desc.format];
	ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = m_desc.mipCount;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 6;

	if (vkCreateImageView(m_logicalDevice, &ivci, NULL, &m_textureView) != VK_SUCCESS)
		return false;

	// Upload initial data.
	if (m_desc.immutable)
	{
		ITexture::Lock lock;
		for (int32_t side = 0; side < 6; ++side)
		{
			for (int32_t mip = 0; mip < m_desc.mipCount; ++mip)
			{
				uint32_t mipSize = getTextureMipPitch(m_desc.format, m_desc.side, m_desc.side, mip);
				
				if (!this->lock(side, mip, lock))
					return false;

				std::memcpy(
					lock.bits,
					m_desc.initialData[side * m_desc.mipCount + mip].data,
					mipSize
				);

				unlock(side, mip);
			}
		}
	}
	return true;
}

void CubeTextureVk::destroy()
{
}

ITexture* CubeTextureVk::resolve()
{
	return this;
}

int32_t CubeTextureVk::getMips() const
{
	return m_desc.mipCount;
}

int32_t CubeTextureVk::getSide() const
{
	return m_desc.side;
}

bool CubeTextureVk::lock(int32_t side, int32_t level, Lock& lock)
{
	uint32_t lockSize = getTextureSize(
		m_desc.format,
		m_desc.side,
		m_desc.side,
		m_desc.mipCount
	);

	// Create staging buffer.
	if (!createBuffer(
		m_physicalDevice,
		m_logicalDevice,
		lockSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_stagingBuffer,
		m_stagingBufferMemory
	))
		return false;

	// Map staging buffer.
	uint8_t* data = nullptr;
	vkMapMemory(
		m_logicalDevice,
		m_stagingBufferMemory,
		0,
		lockSize,
		0,
		(void**)&data
	);

	lock.pitch = m_desc.side * sizeof(uint32_t);
	lock.bits = data;
	return true;
}

void CubeTextureVk::unlock(int32_t side, int32_t level)
{
	// Change layout of texture to be able to copy staging buffer into texture.
	changeImageLayout(
		m_logicalDevice,
		m_setupCommandPool,
		m_setupQueue,
		m_textureImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	// Copy staging buffer into texture.
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(
		m_logicalDevice,
		m_setupCommandPool
	);

	uint32_t mipSide = getTextureMipSize(m_desc.side, level);
	uint32_t mipSize = getTextureMipPitch(m_desc.format, m_desc.side, m_desc.side, level);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = level;
	region.imageSubresource.baseArrayLayer = side;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { mipSide, mipSide, 1 };

	vkCmdCopyBufferToImage(
		commandBuffer,
		m_stagingBuffer,
		m_textureImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	endSingleTimeCommands(
		m_logicalDevice,
		m_setupCommandPool,
		commandBuffer,
		m_setupQueue
	);

	// Change layout of texture to optimal sampling.
	changeImageLayout(
		m_logicalDevice,
		m_setupCommandPool,
		m_setupQueue,
		m_textureImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	// Free staging buffer.
	vkDestroyBuffer(m_logicalDevice, m_stagingBuffer, nullptr);
	vkFreeMemory(m_logicalDevice, m_stagingBufferMemory, nullptr);

	m_stagingBuffer = nullptr;
	m_stagingBufferMemory = nullptr;
}

	}
}
