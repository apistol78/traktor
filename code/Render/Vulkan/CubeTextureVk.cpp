#include <cstring>
#include "Core/Log/Log.h"
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
	VmaAllocator allocator,
	VkCommandPool setupCommandPool,
	VkQueue setupQueue,
	const CubeTextureCreateDesc& desc
)
:	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_allocator(allocator)
,	m_setupCommandPool(setupCommandPool)
,	m_setupQueue(setupQueue)
,	m_desc(desc)
,	m_textureImageAllocation(0)
,	m_textureImage(0)
,	m_textureView(0)
,	m_stagingBufferAllocation(0)
,	m_stagingBuffer(0)
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
	
	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	if (vmaCreateImage(m_allocator, &ici, &aci, &m_textureImage, &m_textureImageAllocation, nullptr) != VK_SUCCESS)
	{
		log::error << L"Failed to create VK cube texture; unable to allocate image memory." << Endl;
		return false;			
	}

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
	if (m_textureImageAllocation != 0)
	{
		vmaFreeMemory(m_allocator, m_textureImageAllocation);
		m_textureImageAllocation = 0;
	}

	if (m_textureImage != 0)
	{
		vkDestroyImage(m_logicalDevice, m_textureImage, 0);
		m_textureImage = 0;
	}
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
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = lockSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	if (vmaCreateBuffer(m_allocator, &bufferInfo, &aci, &m_stagingBuffer, &m_stagingBufferAllocation, nullptr) != VK_SUCCESS)
		return false;	

	// Map staging buffer.
	uint8_t* data = nullptr;
	vmaMapMemory(m_allocator, m_stagingBufferAllocation, (void**)&data);

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
	vkDestroyBuffer(m_logicalDevice, m_stagingBuffer, 0);
	vmaFreeMemory(m_allocator, m_stagingBufferAllocation);

	m_stagingBuffer = 0;
	m_stagingBufferAllocation = 0;
}

bool CubeTextureVk::copy(int32_t side, int32_t level, const ISimpleTexture* sourceTexture)
{
	return false;
}

	}
}
