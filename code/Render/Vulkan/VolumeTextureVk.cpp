#include <cstring>
#include "Core/Misc/TString.h"
#include "Render/Types.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/CommandBufferPool.h"
#include "Render/Vulkan/Context.h"
#include "Render/Vulkan/Queue.h"
#include "Render/Vulkan/UtilitiesVk.h"
#include "Render/Vulkan/VolumeTextureVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureVk", VolumeTextureVk, IVolumeTexture)

VolumeTextureVk::VolumeTextureVk(Context* context)
:	m_context(context)
,	m_textureImage(0)
,	m_textureView(0)
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
	Queue* graphicsQueue,
	CommandBufferPool* graphicsCommandPool,
	const VolumeTextureCreateDesc& desc,
	const wchar_t* const tag
)
{
	if (desc.immutable)
	{
		const VkFormat* vkTextureFormats = desc.sRGB ? c_vkTextureFormats_sRGB : c_vkTextureFormats;
		if (vkTextureFormats[desc.format] == VK_FORMAT_UNDEFINED)
			return false;

		uint32_t imageSize = getTextureSize(desc.format, desc.width, desc.height, 1) * desc.depth;

		// Create staging buffer.
		VkBuffer stagingBuffer = 0;
		VkDeviceMemory stagingBufferMemory = 0;

		if (!createBuffer(
			m_context->getPhysicalDevice(),
			m_context->getLogicalDevice(),
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		))
			return false;

		// Copy data into staging buffer.
		uint8_t* data = nullptr;
		vkMapMemory(m_context->getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, (void**)&data);
		for (int32_t slice = 0; slice < desc.depth; ++slice)
		{
			uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, 0);
			std::memcpy(data, (uint8_t*)desc.initialData[0].data + desc.initialData[0].slicePitch * slice, mipSize);
			data += mipSize;
		}
		vkUnmapMemory(m_context->getLogicalDevice(), stagingBufferMemory);

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
		
		if (vkCreateImage(m_context->getLogicalDevice(), &ici, nullptr, &m_textureImage) != VK_SUCCESS)
			return false;

		// Set debug name of texture.
		setObjectDebugName(m_context->getLogicalDevice(), tag, (uint64_t)m_textureImage, VK_OBJECT_TYPE_IMAGE);

		// Calculate memory requirement of texture image.
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(m_context->getLogicalDevice(), m_textureImage, &memoryRequirements);

		// Allocate texture memory.
		VkDeviceMemory textureImageMemory = 0;

		VkMemoryAllocateInfo mai = {};
		mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mai.allocationSize = memoryRequirements.size;
		mai.memoryTypeIndex = getMemoryTypeIndex(m_context->getPhysicalDevice(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryRequirements);

		if (vkAllocateMemory(m_context->getLogicalDevice(), &mai, nullptr, &textureImageMemory) != VK_SUCCESS)
			return false;

		vkBindImageMemory(m_context->getLogicalDevice(), m_textureImage, textureImageMemory, 0);

		// Create texture view.
		VkImageViewCreateInfo ivci = {};
		ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivci.image = m_textureImage;
		ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
		ivci.format = vkTextureFormats[desc.format];
		ivci.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ivci.subresourceRange.baseMipLevel = 0;
		ivci.subresourceRange.levelCount = desc.mipCount;
		ivci.subresourceRange.baseArrayLayer = 0;
		ivci.subresourceRange.layerCount = 1;
		if (vkCreateImageView(m_context->getLogicalDevice(), &ivci, NULL, &m_textureView) != VK_SUCCESS)
			return false;

		VkCommandBuffer commandBuffer = graphicsCommandPool->acquireAndBegin();

		// Change layout of texture to be able to copy staging buffer into texture.
		VkImageMemoryBarrier imb = {};
		imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imb.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imb.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.image = m_textureImage;
		imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imb.subresourceRange.baseMipLevel = 0;
		imb.subresourceRange.levelCount = 1;
		imb.subresourceRange.baseArrayLayer = 0;
		imb.subresourceRange.layerCount = 1;
		imb.srcAccessMask = 0;
		imb.dstAccessMask = 0;
		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imb
		);

		uint32_t offset = 0;
		for (int32_t slice = 0; slice < desc.depth; ++slice)
		{
			// Copy staging buffer into texture.
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

		// Change layout of texture to optimal sampling.
		imb = {};
		imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imb.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imb.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.image = m_textureImage;
		imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imb.subresourceRange.baseMipLevel = 0;
		imb.subresourceRange.levelCount = 1;
		imb.subresourceRange.baseArrayLayer = 0;
		imb.subresourceRange.layerCount = 1;
		imb.srcAccessMask = 0;
		imb.dstAccessMask = 0;
		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imb
		);

		// End recording command buffer.
		vkEndCommandBuffer(commandBuffer);

		// Submit and wait for commands to execute.
		VkSubmitInfo si = {};
		si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		si.commandBufferCount = 1;
		si.pCommandBuffers = &commandBuffer;
		graphicsQueue->submitAndWait(si);

		graphicsCommandPool->release(commandBuffer);

		// Free staging buffer.
		vkDestroyBuffer(m_context->getLogicalDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_context->getLogicalDevice(), stagingBufferMemory, nullptr);
	}

	m_width = desc.width;
	m_height = desc.height;
	m_depth = desc.depth;
	return true;
}

void VolumeTextureVk::destroy()
{
	if (m_context)
	{
		m_context->addDeferredCleanup([=](Context* cx) {

			// \fixme

		});
	}
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
