/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vulkan/TextureVk.h"

#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Atomic.h"
#include "Render/Types.h"
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Image.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

#include <cstring>

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureVk", TextureVk, ITexture)

TextureVk::TextureVk(Context* context, uint32_t& instances)
	: m_context(context)
	, m_instances(instances)
{
	Atomic::increment((int32_t&)m_instances);
}

TextureVk::~TextureVk()
{
	safeDestroy(m_stagingBuffer);
	safeDestroy(m_textureImage);
	m_context = nullptr;

	Atomic::decrement((int32_t&)m_instances);
}

bool TextureVk::create(
	const SimpleTextureCreateDesc& desc,
	const wchar_t* const tag)
{
	const VkFormat* vkTextureFormats = desc.sRGB ? c_vkTextureFormats_sRGB : c_vkTextureFormats;
	if (vkTextureFormats[desc.format] == VK_FORMAT_UNDEFINED)
	{
		log::error << L"Failed to create 2D texture; unsupported format (\"" << getTextureFormatName(desc.format) << L"\" (" << (int)desc.format << L"), " << (desc.sRGB ? L"sRGB" : L"linear") << L")." << Endl;
		return false;
	}

	uint32_t usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	if (desc.shaderStorage)
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;

	// Create image.
	m_textureImage = new Image(m_context);
	if (!m_textureImage->createSimple(
			desc.width,
			desc.height,
			desc.mipCount,
			vkTextureFormats[desc.format],
			usageFlags))
	{
		m_textureImage = nullptr;
		return false;
	}

	// Create staging buffer.
	const uint32_t imageSize = getTextureSize(desc.format, desc.width, desc.height, desc.mipCount);

	m_stagingBuffer = new ApiBuffer(m_context);
	if (!m_stagingBuffer->create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true))
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

	// Layout the deferred upload leaves the image in, i.e. the layout consumers see.
	m_restingLayout = desc.shaderStorage ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	Ref< ITexture > self = this;
	m_context->addDeferredUpload(
		[desc, self, this](Context* cx, CommandBuffer* commandBuffer) {

			if (!m_textureImage)
				return;

			// Explicit as the tracked layout was advanced when queued; the image is still undefined here.
			m_textureImage->changeLayoutExplicit(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, desc.mipCount, 0, 1);

			// Copy staging buffer into texture.
			uint32_t offset = 0;
			for (int32_t mip = 0; mip < desc.mipCount; ++mip)
			{
				const uint32_t mipWidth = getTextureMipSize(desc.width, mip);
				const uint32_t mipHeight = getTextureMipSize(desc.height, mip);
				const uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, mip);

				const VkBufferImageCopy region = {
					.bufferOffset = offset,
					.bufferRowLength = 0,
					.bufferImageHeight = 0,
					.imageSubresource = {
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.mipLevel = (uint32_t)mip,
						.baseArrayLayer = 0,
						.layerCount = 1 },
					.imageOffset = { 0, 0, 0 },
					.imageExtent = { mipWidth, mipHeight, 1 }
				};

				vkCmdCopyBufferToImage(
					*commandBuffer,
					*m_stagingBuffer,
					m_textureImage->getVkImage(),
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&region);

				offset += mipSize;
			}

			// Change layout of texture to its resting layout.
			m_textureImage->changeLayoutExplicit(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_restingLayout, VK_IMAGE_ASPECT_COLOR_BIT, 0, desc.mipCount, 0, 1);

			// Mutable textures keep the staging buffer for updates, thus have to know when it is read.
			if (desc.immutable)
				safeDestroy(m_stagingBuffer);
			else
				stagingRead(commandBuffer);
		},
		desc.immutable ? imageSize : 0);

	// Track the layout the queued upload will establish; it executes before any work
	// consuming this texture, so the image must not read as UNDEFINED.
	m_textureImage->setVkImageLayout(m_restingLayout, 0, desc.mipCount, 0, 1);

	m_size = { desc.width, desc.height, 1, desc.mipCount };
	m_format = desc.format;
	return true;
}

bool TextureVk::create(
	const CubeTextureCreateDesc& desc,
	const wchar_t* const tag)
{
	const VkFormat* vkTextureFormats = desc.sRGB ? c_vkTextureFormats_sRGB : c_vkTextureFormats;
	if (vkTextureFormats[desc.format] == VK_FORMAT_UNDEFINED)
	{
		log::error << L"Failed to create cube texture; unsupported format (\"" << getTextureFormatName(desc.format) << L"\" (" << (int)desc.format << L"), " << (desc.sRGB ? L"sRGB" : L"linear") << L")." << Endl;
		return false;
	}

	uint32_t usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	if (desc.shaderStorage)
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;

	// Create image.
	m_textureImage = new Image(m_context);
	if (!m_textureImage->createCube(
			desc.side,
			desc.side,
			desc.mipCount,
			vkTextureFormats[desc.format],
			usageFlags))
	{
		m_textureImage = nullptr;
		return false;
	}

	// Create staging buffer.
	const uint32_t imageSize = getTextureSize(desc.format, desc.side, desc.side, desc.mipCount) * 6;

	m_stagingBuffer = new ApiBuffer(m_context);
	if (!m_stagingBuffer->create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true))
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

	m_restingLayout = desc.shaderStorage ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// Copy staging buffer into texture as a deferred upload.
	Ref< ITexture > self = this;
	m_context->addDeferredUpload(
		[desc, self, this](Context* cx, CommandBuffer* commandBuffer) {
			if (!m_textureImage)
				return;

			m_textureImage->changeLayoutExplicit(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, desc.mipCount, 0, 6);

			uint32_t offset = 0;
			for (int32_t side = 0; side < 6; ++side)
			{
				for (int32_t mip = 0; mip < desc.mipCount; ++mip)
				{
					const uint32_t mipSide = getTextureMipSize(desc.side, mip);
					const uint32_t mipSize = getTextureMipPitch(desc.format, desc.side, desc.side, mip);

					const VkBufferImageCopy region = {
						.bufferOffset = offset,
						.bufferRowLength = 0,
						.bufferImageHeight = 0,
						.imageSubresource = {
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.mipLevel = (uint32_t)mip,
							.baseArrayLayer = (uint32_t)side,
							.layerCount = 1,
						},
						.imageOffset = { 0, 0, 0 },
						.imageExtent = { mipSide, mipSide, 1 }
					};

					vkCmdCopyBufferToImage(
						*commandBuffer,
						*m_stagingBuffer,
						m_textureImage->getVkImage(),
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1,
						&region);

					offset += mipSize;
				}
			}

			m_textureImage->changeLayoutExplicit(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_restingLayout, VK_IMAGE_ASPECT_COLOR_BIT, 0, desc.mipCount, 0, 6);

			if (desc.immutable)
				safeDestroy(m_stagingBuffer);
			else
				stagingRead(commandBuffer);
		},
		desc.immutable ? imageSize : 0);

	m_textureImage->setVkImageLayout(m_restingLayout, 0, desc.mipCount, 0, 6);

	m_size = { desc.side, desc.side, 1, desc.mipCount };
	m_sideCount = 6;
	m_format = desc.format;
	return true;
}

bool TextureVk::create(
	const VolumeTextureCreateDesc& desc,
	const wchar_t* const tag)
{
	const VkFormat* vkTextureFormats = desc.sRGB ? c_vkTextureFormats_sRGB : c_vkTextureFormats;
	if (vkTextureFormats[desc.format] == VK_FORMAT_UNDEFINED)
	{
		log::error << L"Failed to create volume texture; unsupported format (\"" << getTextureFormatName(desc.format) << L"\" (" << (int)desc.format << L"), " << (desc.sRGB ? L"sRGB" : L"linear") << L")." << Endl;
		return false;
	}

	uint32_t usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	if (desc.shaderStorage)
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;

	// Create image.
	m_textureImage = new Image(m_context);
	if (!m_textureImage->createVolume(
			desc.width,
			desc.height,
			desc.depth,
			desc.mipCount,
			vkTextureFormats[desc.format],
			usageFlags))
	{
		m_textureImage = nullptr;
		return false;
	}

	// Every mip is uploaded with its own depth; initial data holds one entry per mip, slices slicePitch apart.
	const int32_t mipCount = std::max< int32_t >(desc.mipCount, 1);
	uint32_t imageSize = 0;
	for (int32_t mip = 0; mip < mipCount; ++mip)
	{
		const uint32_t mipDepth = getTextureMipSize(desc.depth, mip);
		imageSize += getTextureMipPitch(desc.format, desc.width, desc.height, mip) * mipDepth;
	}

	// Create staging buffer.
	m_stagingBuffer = new ApiBuffer(m_context);
	if (!m_stagingBuffer->create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, true))
		return false;

	// Copy data into staging buffer; slices tightly packed within each mip.
	uint8_t* data = (uint8_t*)m_stagingBuffer->lock();
	if (!data)
		return false;
	for (int32_t mip = 0; mip < mipCount; ++mip)
	{
		const uint32_t mipDepth = getTextureMipSize(desc.depth, mip);
		const uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, mip);
		const TextureInitialData& initialData = desc.initialData[mip];
		for (uint32_t slice = 0; slice < mipDepth; ++slice)
		{
			if (desc.immutable && initialData.data != nullptr)
				std::memcpy(data, (const uint8_t*)initialData.data + initialData.slicePitch * slice, mipSize);
			else
				std::memset(data, 0, mipSize);
			data += mipSize;
		}
	}
	m_stagingBuffer->unlock();

	m_restingLayout = desc.shaderStorage ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// Copy staging buffer into texture as a deferred upload.
	Ref< ITexture > self = this;
	m_context->addDeferredUpload(
		[desc, mipCount, self, this](Context* cx, CommandBuffer* commandBuffer) {
			if (!m_textureImage)
				return;

			m_textureImage->changeLayoutExplicit(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, mipCount, 0, 1);

			uint32_t offset = 0;
			for (int32_t mip = 0; mip < mipCount; ++mip)
			{
				const uint32_t mipWidth = getTextureMipSize(desc.width, mip);
				const uint32_t mipHeight = getTextureMipSize(desc.height, mip);
				const uint32_t mipDepth = getTextureMipSize(desc.depth, mip);
				const uint32_t mipSize = getTextureMipPitch(desc.format, desc.width, desc.height, mip);

				const VkBufferImageCopy region = {
					.bufferOffset = offset,
					.bufferRowLength = 0,
					.bufferImageHeight = 0,
					.imageSubresource = {
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.mipLevel = (uint32_t)mip,
						.baseArrayLayer = 0,
						.layerCount = 1 },
					.imageOffset = { 0, 0, 0 },
					.imageExtent = { mipWidth, mipHeight, mipDepth }
				};

				vkCmdCopyBufferToImage(
					*commandBuffer,
					*m_stagingBuffer,
					m_textureImage->getVkImage(),
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&region);

				offset += mipSize * mipDepth;
			}

			m_textureImage->changeLayoutExplicit(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_restingLayout, VK_IMAGE_ASPECT_COLOR_BIT, 0, mipCount, 0, 1);

			if (desc.immutable)
				safeDestroy(m_stagingBuffer);
			else
				stagingRead(commandBuffer);
		},
		desc.immutable ? imageSize : 0);

	m_textureImage->setVkImageLayout(m_restingLayout, 0, mipCount, 0, 1);

	m_size = { desc.width, desc.height, desc.depth, desc.mipCount };
	m_format = desc.format;
	return true;
}

void TextureVk::destroy()
{
	// Only relinquish ownership; pending renders may still use the texture.
}

ITexture* TextureVk::resolve()
{
	return this;
}

ITexture::Size TextureVk::getSize() const
{
	return m_size;
}

int32_t TextureVk::getBindlessIndex() const
{
	return (int32_t)m_textureImage->getSampledResourceIndex();
}

bool TextureVk::lock(int32_t side, int32_t level, Lock& lock)
{
	if (m_stagingBuffer == nullptr)
		return false;
	if (side < 0 || side >= m_sideCount || level < 0 || level >= m_size.mips)
		return false;

	// Wait until no submitted copy reads the staging buffer, then hold the staging lock until unlock.
	for (;;)
	{
		m_stagingLock.wait();
		const uint64_t stagingEpoch = m_stagingEpoch;
		if (stagingEpoch == 0 || m_context->getCompletedEpoch() >= stagingEpoch)
			break;
		m_stagingLock.release();
		m_context->waitForEpoch(stagingEpoch);
	}

	uint8_t* bits = (uint8_t*)m_stagingBuffer->lock();
	if (!bits)
	{
		m_stagingLock.release();
		return false;
	}

	lock.bits = bits + getStagingOffset(side, level);
	lock.pitch = getTextureRowPitch(m_format, m_size.x, level);
	return true;
}

void TextureVk::unlock(int32_t side, int32_t level)
{
	const Region region = {
		0,
		0,
		(int32_t)getTextureMipSize(m_size.x, level),
		(int32_t)getTextureMipSize(m_size.y, level)
	};
	unlock(side, level, region);
}

void TextureVk::unlock(int32_t side, int32_t level, const Region& region)
{
	m_stagingBuffer->unlock();
	m_stagingLock.release();

	const int32_t mipWidth = (int32_t)getTextureMipSize(m_size.x, level);
	const int32_t mipHeight = (int32_t)getTextureMipSize(m_size.y, level);

	int32_t x0 = clamp(region.x, 0, mipWidth);
	int32_t y0 = clamp(region.y, 0, mipHeight);
	int32_t x1 = clamp(region.x + region.width, 0, mipWidth);
	int32_t y1 = clamp(region.y + region.height, 0, mipHeight);

	// Nothing modified since lock; no transfer necessary.
	if (x0 >= x1 || y0 >= y1)
		return;

	const uint32_t rowPitch = getTextureRowPitch(m_format, m_size.x, level);
	const uint32_t texelSize = getTextureBlockSize(m_format);

	// Buffer offset must be a multiple of both 4 and the texel size, so align the region out.
	// Block compressed formats are transferred whole since the staging layout is in blocks.
	const bool partial = (getTextureBlockDenom(m_format) == 1 && (rowPitch % 4) == 0);
	if (partial)
	{
		const int32_t alignX = (texelSize >= 4) ? 1 : (int32_t)(4 / texelSize);
		x0 = (x0 / alignX) * alignX;
	}
	else
	{
		x0 = 0;
		y0 = 0;
		x1 = mipWidth;
		y1 = mipHeight;
	}

	const VkBufferImageCopy copyRegion = {
		.bufferOffset = getStagingOffset(side, level) + (partial ? ((uint32_t)y0 * rowPitch + (uint32_t)x0 * texelSize) : 0),
		.bufferRowLength = partial ? (rowPitch / texelSize) : 0,
		.bufferImageHeight = 0,
		.imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = (uint32_t)level,
			.baseArrayLayer = (uint32_t)side,
			.layerCount = 1 },
		.imageOffset = { x0, y0, 0 },
		.imageExtent = { (uint32_t)(x1 - x0), (uint32_t)(y1 - y0), 1 }
	};

	// The image is in its resting layout whenever uploads execute; the tracked layout is left as-is.
	Ref< ITexture > self = this;
	m_context->addDeferredUpload(
		[copyRegion, level, side, self, this](Context* cx, CommandBuffer* commandBuffer) {
			if (!m_textureImage || !m_stagingBuffer)
				return;

			T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_stagingLock);

			m_textureImage->changeLayoutExplicit(commandBuffer, m_restingLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, level, 1, side, 1);

			vkCmdCopyBufferToImage(
				*commandBuffer,
				*m_stagingBuffer,
				m_textureImage->getVkImage(),
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&copyRegion);

			m_textureImage->changeLayoutExplicit(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_restingLayout, VK_IMAGE_ASPECT_COLOR_BIT, level, 1, side, 1);

			m_stagingEpoch = commandBuffer->getSubmissionEpoch();
		});
}

void TextureVk::stagingRead(CommandBuffer* commandBuffer)
{
	T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_stagingLock);
	m_stagingEpoch = commandBuffer->getSubmissionEpoch();
}

uint32_t TextureVk::getStagingOffset(int32_t side, int32_t level) const
{
	uint32_t sideSize = 0;
	uint32_t levelOffset = 0;
	for (int32_t mip = 0; mip < m_size.mips; ++mip)
	{
		const uint32_t mipDepth = getTextureMipSize(m_size.z, mip);
		const uint32_t mipSize = getTextureMipPitch(m_format, m_size.x, m_size.y, mip) * mipDepth;
		if (mip < level)
			levelOffset += mipSize;
		sideSize += mipSize;
	}
	return (uint32_t)side * sideSize + levelOffset;
}

}
