#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/Types.h"
#include "Render/Vulkan/SimpleTextureVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/Buffer.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Image.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureVk", SimpleTextureVk, ISimpleTexture)

SimpleTextureVk::SimpleTextureVk(Context* context, uint32_t& instances)
:	m_context(context)
,	m_instances(instances)
{
	Atomic::increment((int32_t&)m_instances);
}

SimpleTextureVk::~SimpleTextureVk()
{
	destroy();
	Atomic::decrement((int32_t&)m_instances);
}

bool SimpleTextureVk::create(
	const SimpleTextureCreateDesc& desc,
	const wchar_t* const tag
)
{
	const VkFormat* vkTextureFormats = desc.sRGB ? c_vkTextureFormats_sRGB : c_vkTextureFormats;
	if (vkTextureFormats[desc.format] == VK_FORMAT_UNDEFINED)
	{
		log::error << L"Failed to create 2D texture; unsupported format (\"" << getTextureFormatName(desc.format) << L"\" (" << (int)desc.format << L"), " << (desc.sRGB ? L"sRGB" : L"linear") << L")." << Endl;
		return false;
	}

	// Create image.
	m_textureImage = new Image(m_context);
	if (!m_textureImage->createSimple(
		desc.width,
		desc.height,
		desc.mipCount,
		vkTextureFormats[desc.format],
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
	))
	{
		m_textureImage = nullptr;
		return false;
	}

	uint32_t imageSize = getTextureSize(desc.format, desc.width, desc.height, desc.mipCount);

	// Create staging buffer.
	m_stagingBuffer = new Buffer(m_context);
	m_stagingBuffer->create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true);

	// Copy data into staging buffer.
	uint8_t* data = (uint8_t*)m_stagingBuffer->lock();
	if (!data)
		return false;
	for (int32_t mip = 0; mip < desc.mipCount; ++mip)
	{
		uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, mip);
		if (desc.immutable)
			std::memcpy(data, desc.initialData[mip].data, mipSize);
		else
			std::memset(data, 0, mipSize);
		data += mipSize;
	}
	m_stagingBuffer->unlock();

	m_dirty = true;
	m_desc = desc;
	return true;
}

void SimpleTextureVk::destroy()
{
	m_context = nullptr;
	safeDestroy(m_stagingBuffer);
	safeDestroy(m_textureImage);
}

ITexture* SimpleTextureVk::resolve()
{
	// Update texture if staging buffer contain new data.
	if (m_dirty)
	{
		auto commandBuffer = m_context->getGraphicsQueue()->acquireCommandBuffer(T_FILE_LINE_W);

		// Change layout of texture to be able to copy staging buffer into texture.
		VkImageMemoryBarrier imb = {};
		imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imb.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imb.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.image = m_textureImage->getVkImage();
		imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imb.subresourceRange.baseMipLevel = 0;
		imb.subresourceRange.levelCount = m_desc.mipCount;
		imb.subresourceRange.baseArrayLayer = 0;
		imb.subresourceRange.layerCount = 1;
		imb.srcAccessMask = 0;
		imb.dstAccessMask = 0;
		vkCmdPipelineBarrier(
			*commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imb
		);

		// Copy staging buffer into texture.
		uint32_t offset = 0;
		for (int32_t mip = 0; mip < m_desc.mipCount; ++mip)
		{
			uint32_t mipWidth = getTextureMipSize(m_desc.width, mip);
			uint32_t mipHeight = getTextureMipSize(m_desc.height, mip);
			uint32_t mipSize = getTextureMipPitch(m_desc.format, m_desc.width, m_desc.height, mip);

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
				*commandBuffer,
				*m_stagingBuffer,
				m_textureImage->getVkImage(),
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
		imb.image = m_textureImage->getVkImage();
		imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imb.subresourceRange.baseMipLevel = 0;
		imb.subresourceRange.levelCount = m_desc.mipCount;
		imb.subresourceRange.baseArrayLayer = 0;
		imb.subresourceRange.layerCount = 1;
		imb.srcAccessMask = 0;
		imb.dstAccessMask = 0;
		vkCmdPipelineBarrier(
			*commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imb
		);

		commandBuffer->submitAndWait();

		// Free staging buffer if immutable, no longer
		// allowed to update texture.
		if (m_desc.immutable)
			safeDestroy(m_stagingBuffer);

		m_dirty = false;
	}
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
	if (m_stagingBuffer != nullptr)
	{
		lock.bits = m_stagingBuffer->lock();
		lock.pitch = getTextureRowPitch(m_desc.format, m_desc.width, level);
		return true;
	}
	else
		return false;
}

void SimpleTextureVk::unlock(int32_t level)
{
	m_stagingBuffer->unlock();
	m_dirty = true;
}

void* SimpleTextureVk::getInternalHandle()
{
	return nullptr;
}

	}
}
