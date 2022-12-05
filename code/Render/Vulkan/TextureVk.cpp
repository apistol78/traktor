/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Atomic.h"
#include "Render/Types.h"
#include "Render/Vulkan/TextureVk.h"
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Image.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureVk", TextureVk, ITexture)

TextureVk::TextureVk(Context* context, uint32_t& instances)
:	m_context(context)
,	m_instances(instances)
{
	Atomic::increment((int32_t&)m_instances);
}

TextureVk::~TextureVk()
{
	destroy();
	Atomic::decrement((int32_t&)m_instances);
}

bool TextureVk::create(
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
	if (desc.immutable)
		safeDestroy(m_stagingBuffer);

	m_size = { desc.width, desc.height, 1, desc.mipCount };
	m_format = desc.format;
	return true;
}

bool TextureVk::create(
	const CubeTextureCreateDesc& desc,
	const wchar_t* const tag
)
{
	const VkFormat* vkTextureFormats = desc.sRGB ? c_vkTextureFormats_sRGB : c_vkTextureFormats;
	if (vkTextureFormats[desc.format] == VK_FORMAT_UNDEFINED)
	{
		log::error << L"Failed to create cube texture; unsupported format (\"" << getTextureFormatName(desc.format) << L"\" (" << (int)desc.format << L"), " << (desc.sRGB ? L"sRGB" : L"linear") << L")." << Endl;
		return false;
	}

	// Create image.
	m_textureImage = new Image(m_context);
	if (!m_textureImage->createCube(
		desc.side,
		desc.side,
		desc.mipCount,
		vkTextureFormats[desc.format],
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
	))
	{
		m_textureImage = nullptr;
		return false;
	}

	// Create staging buffer.
	const uint32_t imageSize = getTextureSize(desc.format, desc.side, desc.side, desc.mipCount) * 6;

	m_stagingBuffer = new ApiBuffer(m_context);
	m_stagingBuffer->create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true);

	// Upload initial data.
	auto commandBuffer = m_context->getGraphicsQueue()->acquireCommandBuffer(T_FILE_LINE_W);
	if (!commandBuffer)
		return false;

	uint8_t* bits = (uint8_t*)m_stagingBuffer->lock();
	if (!bits)
		return false;

	for (int32_t side = 0; side < 6; ++side)
	{
		for (int32_t mip = 0; mip < desc.mipCount; ++mip)
		{
			const uint32_t mipSize = getTextureMipPitch(desc.format, desc.side, desc.side, mip);

			if (desc.immutable)
				std::memcpy(bits, desc.initialData[side * desc.mipCount + mip].data, mipSize);
			else
				std::memset(bits, 0, mipSize);
		
			bits += mipSize;
		}
	}

	m_stagingBuffer->unlock();

	// Change layout of texture to be able to copy staging buffer into texture.
	m_textureImage->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, desc.mipCount, 0, 6);

	// Copy staging buffer into texture.
	uint32_t offset = 0;
	for (int32_t side = 0; side < 6; ++side)
	{
		for (int32_t mip = 0; mip < desc.mipCount; ++mip)
		{
			const uint32_t mipSide = getTextureMipSize(desc.side, mip);
			const uint32_t mipSize = getTextureMipPitch(desc.format, desc.side, desc.side, mip);

			VkBufferImageCopy region = {};
			region.bufferOffset = offset;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = mip;
			region.imageSubresource.baseArrayLayer = side;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { mipSide, mipSide, 1 };

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
	}

	// Change layout of texture to optimal sampling.
	m_textureImage->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, desc.mipCount, 0, 6);

	commandBuffer->submitAndWait();

	// Free staging buffer if immutable, no longer
	// allowed to update texture.
	if (desc.immutable)
		safeDestroy(m_stagingBuffer);

	m_size = { desc.side, desc.side, 1, desc.mipCount };
	m_format = desc.format;
	return true;
}

bool TextureVk::create(
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

	const uint32_t imageSize = getTextureSize(desc.format, desc.width, desc.height, 1) * desc.depth;

	// Create staging buffer.
	m_stagingBuffer = new ApiBuffer(m_context);
	m_stagingBuffer->create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true);

	// Copy data into staging buffer.
	uint8_t* data = (uint8_t*)m_stagingBuffer->lock();
	if (!data)
		return false;
	for (int32_t slice = 0; slice < desc.depth; ++slice)
	{
		const uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, 0);
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
		const uint32_t mipWidth = getTextureMipSize(desc.width, 0);
		const uint32_t mipHeight = getTextureMipSize(desc.height, 0);
		const uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, 0);

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

	m_size = { desc.width, desc.height, desc.depth, desc.mipCount };
	m_format = desc.format;
	return true;
}

void TextureVk::destroy()
{
	m_context = nullptr;
	safeDestroy(m_stagingBuffer);
	safeDestroy(m_textureImage);
}

ITexture* TextureVk::resolve()
{
	return this;
}

ITexture::Size TextureVk::getSize() const
{
	return m_size;
}

bool TextureVk::lock(int32_t side, int32_t level, Lock& lock)
{
	if (m_stagingBuffer != nullptr)
	{
		lock.bits = m_stagingBuffer->lock();
		lock.pitch = getTextureRowPitch(m_format, m_size.x, level);
		return true;
	}
	else
		return false;
}

void TextureVk::unlock(int32_t side, int32_t level)
{
	m_stagingBuffer->unlock();
	
	auto commandBuffer = m_context->getGraphicsQueue()->acquireCommandBuffer(T_FILE_LINE_W);

	// Change layout of texture to be able to copy staging buffer into texture.
	m_textureImage->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, level, 1, 0, 1);

	// Copy staging buffer into texture.
	const uint32_t mipWidth = getTextureMipSize(m_size.x, level);
	const uint32_t mipHeight = getTextureMipSize(m_size.y, level);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = level;
	region.imageSubresource.baseArrayLayer = side;
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

}
