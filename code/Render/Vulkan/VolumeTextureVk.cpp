#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Atomic.h"
#include "Render/Types.h"
#include "Render/Vulkan/VolumeTextureVk.h"
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Image.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureVk", VolumeTextureVk, IVolumeTexture)

VolumeTextureVk::VolumeTextureVk(Context* context, uint32_t& instances)
:	m_context(context)
,	m_instances(instances)
{
	Atomic::increment((int32_t&)m_instances);
}

VolumeTextureVk::~VolumeTextureVk()
{
	destroy();
	Atomic::decrement((int32_t&)m_instances);
}

bool VolumeTextureVk::create(
	const VolumeTextureCreateDesc& desc,
	const wchar_t* const tag
)
{
	const VkFormat* vkTextureFormats = desc.sRGB ? c_vkTextureFormats_sRGB : c_vkTextureFormats;
	if (vkTextureFormats[desc.format] == VK_FORMAT_UNDEFINED)
	{
		log::error << L"Failed to create volume texture; unsupported format (\"" << getTextureFormatName(desc.format) << L"\" (" << (int)desc.format << L"), " << (desc.sRGB ? L"sRGB" : L"linear") << L")." << Endl;
		return false;
	}

	// Create image.
	m_textureImage = new Image(m_context);
	if (!m_textureImage->createVolume(
		desc.width,
		desc.height,
		desc.depth,
		desc.mipCount,
		vkTextureFormats[desc.format],
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
	))
	{
		m_textureImage = nullptr;
		return false;
	}

	uint32_t imageSize = getTextureSize(desc.format, desc.width, desc.height, 1) * desc.depth;

	// Create staging buffer.
	m_stagingBuffer = new ApiBuffer(m_context);
	m_stagingBuffer->create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true);

	// Copy data into staging buffer.
	uint8_t* data = (uint8_t*)m_stagingBuffer->lock();
	if (!data)
		return false;
	for (int32_t slice = 0; slice < desc.depth; ++slice)
	{
		uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, 0);
		if (desc.immutable)
			std::memcpy(data, (uint8_t*)desc.initialData[0].data + desc.initialData[0].slicePitch * slice, mipSize);
		else
			std::memset(data, 0, mipSize);
		data += mipSize;
	}
	m_stagingBuffer->unlock();

	auto commandBuffer = m_context->getGraphicsQueue()->acquireCommandBuffer(T_FILE_LINE_W);

	// Change layout of texture to be able to copy staging buffer into texture.
	m_textureImage->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

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
	m_textureImage->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

	commandBuffer->submitAndWait();

	// Free staging buffer if immutable.
	if (desc.immutable)
		safeDestroy(m_stagingBuffer);

	m_desc = desc;
	return true;
}

void VolumeTextureVk::destroy()
{
	m_context = nullptr;
	safeDestroy(m_stagingBuffer);
	safeDestroy(m_textureImage);
}

ITexture* VolumeTextureVk::resolve()
{
	return this;
}

int32_t VolumeTextureVk::getWidth() const
{
	return m_desc.width;
}

int32_t VolumeTextureVk::getHeight() const
{
	return m_desc.height;
}

int32_t VolumeTextureVk::getDepth() const
{
	return m_desc.depth;
}

int32_t VolumeTextureVk::getMips() const
{
	return m_desc.mipCount;
}

}
