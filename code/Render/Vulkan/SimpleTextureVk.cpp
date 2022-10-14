#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Atomic.h"
#include "Render/Types.h"
#include "Render/Vulkan/SimpleTextureVk.h"
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Image.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
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

	m_desc = desc;

	// Create staging buffer.
	const uint32_t imageSize = getTextureSize(desc.format, desc.width, desc.height, desc.mipCount);

	m_stagingBuffer = new ApiBuffer(m_context);
	m_stagingBuffer->create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true);

	// Upload initial data.
	auto commandBuffer = m_context->getGraphicsQueue()->acquireCommandBuffer(T_FILE_LINE_W);
	if (!commandBuffer)
		return false;

	uint8_t* bits = (uint8_t*)m_stagingBuffer->lock();
	if (!bits)
		return false;

	for (int32_t mip = 0; mip < desc.mipCount; ++mip)
	{
		const uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, mip);

		if (desc.immutable)
			std::memcpy(bits, desc.initialData[mip].data, mipSize);
		else
			std::memset(bits, 0, mipSize);

		bits += mipSize;
	}

	m_stagingBuffer->unlock();
		
	// Change layout of texture to be able to copy staging buffer into texture.
	m_textureImage->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, desc.mipCount, 0, 1);

	// Copy staging buffer into texture.
	uint32_t offset = 0;
	for (int32_t mip = 0; mip < desc.mipCount; ++mip)
	{
		const uint32_t mipWidth = getTextureMipSize(desc.width, mip);
		const uint32_t mipHeight = getTextureMipSize(desc.height, mip);
		const uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, mip);

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
	m_textureImage->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, desc.mipCount, 0, 1);

	// Submit command buffer to perform transfer of stage to texture.
	commandBuffer->submitAndWait();

	// Free staging buffer if immutable, no longer
	// allowed to update texture.
	if (m_desc.immutable)
		safeDestroy(m_stagingBuffer);

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
	return this;
}

int32_t SimpleTextureVk::getWidth() const
{
	return m_desc.width;
}

int32_t SimpleTextureVk::getHeight() const
{
	return m_desc.height;
}

int32_t SimpleTextureVk::getMips() const
{
	return m_desc.mipCount;
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
	
	auto commandBuffer = m_context->getGraphicsQueue()->acquireCommandBuffer(T_FILE_LINE_W);

	// Change layout of texture to be able to copy staging buffer into texture.
	m_textureImage->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, level, 1, 0, 1);

	// Copy staging buffer into texture.
	uint32_t mipWidth = getTextureMipSize(m_desc.width, level);
	uint32_t mipHeight = getTextureMipSize(m_desc.height, level);

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
		*commandBuffer,
		*m_stagingBuffer,
		m_textureImage->getVkImage(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	// Change layout of texture to optimal sampling.
	m_textureImage->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, level, 1, 0, 1);

	commandBuffer->submitAndWait();
}

void* SimpleTextureVk::getInternalHandle()
{
	return nullptr;
}

}
