#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Atomic.h"
#include "Render/Types.h"
#include "Render/Vulkan/CubeTextureVk.h"
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Image.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/Utilities.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubeTextureVk", CubeTextureVk, ICubeTexture)

CubeTextureVk::CubeTextureVk(Context* context, const CubeTextureCreateDesc& desc, uint32_t& instances)
:	m_context(context)
,	m_instances(instances)
,	m_desc(desc)
{
	Atomic::increment((int32_t&)m_instances);
}

CubeTextureVk::~CubeTextureVk()
{
	destroy();
	Atomic::decrement((int32_t&)m_instances);
}

bool CubeTextureVk::create(const wchar_t* const tag)
{
	const VkFormat* vkTextureFormats = m_desc.sRGB ? c_vkTextureFormats_sRGB : c_vkTextureFormats;
	if (vkTextureFormats[m_desc.format] == VK_FORMAT_UNDEFINED)
	{
		log::error << L"Failed to create cube texture; unsupported format (\"" << getTextureFormatName(m_desc.format) << L"\" (" << (int)m_desc.format << L"), " << (m_desc.sRGB ? L"sRGB" : L"linear") << L")." << Endl;
		return false;
	}

	// Create image.
	m_textureImage = new Image(m_context);
	if (!m_textureImage->createCube(
		m_desc.side,
		m_desc.side,
		m_desc.mipCount,
		vkTextureFormats[m_desc.format],
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
	))
	{
		m_textureImage = nullptr;
		return false;
	}

	// Create staging buffer.
	const uint32_t imageSize = getTextureSize(m_desc.format, m_desc.side, m_desc.side, m_desc.mipCount) * 6;

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
		for (int32_t mip = 0; mip < m_desc.mipCount; ++mip)
		{
			const uint32_t mipSize = getTextureMipPitch(m_desc.format, m_desc.side, m_desc.side, mip);

			if (m_desc.immutable)
				std::memcpy(bits, m_desc.initialData[side * m_desc.mipCount + mip].data, mipSize);
			else
				std::memset(bits, 0, mipSize);
		
			bits += mipSize;
		}
	}

	m_stagingBuffer->unlock();

	// Change layout of texture to be able to copy staging buffer into texture.
	m_textureImage->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, m_desc.mipCount, 0, 6);

	// Copy staging buffer into texture.
	uint32_t offset = 0;
	for (int32_t side = 0; side < 6; ++side)
	{
		for (int32_t mip = 0; mip < m_desc.mipCount; ++mip)
		{
			const uint32_t mipSide = getTextureMipSize(m_desc.side, mip);
			const uint32_t mipSize = getTextureMipPitch(m_desc.format, m_desc.side, m_desc.side, mip);

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
	m_textureImage->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, m_desc.mipCount, 0, 6);

	commandBuffer->submitAndWait();

	// Free staging buffer if immutable, no longer
	// allowed to update texture.
	if (m_desc.immutable)
		safeDestroy(m_stagingBuffer);

	return true;
}

void CubeTextureVk::destroy()
{
	m_context = nullptr;
	safeDestroy(m_stagingBuffer);
	safeDestroy(m_textureImage);
}

ITexture* CubeTextureVk::resolve()
{
	return this;
}

int32_t CubeTextureVk::getSide() const
{
	return m_desc.side;
}

int32_t CubeTextureVk::getMips() const
{
	return m_desc.mipCount;
}

bool CubeTextureVk::lock(int32_t side, int32_t level, Lock& lock)
{
	if (m_stagingBuffer != nullptr)
	{
		lock.bits = m_stagingBuffer->lock();
		lock.pitch = getTextureRowPitch(m_desc.format, m_desc.side, level);
		return true;
	}
	else
		return false;
}

void CubeTextureVk::unlock(int32_t side, int32_t level)
{
	m_stagingBuffer->unlock();

	auto commandBuffer = m_context->getGraphicsQueue()->acquireCommandBuffer(T_FILE_LINE_W);

	// Change layout of texture to be able to copy staging buffer into texture.
	m_textureImage->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, level, 1, side, 1);

	// Copy staging buffer into texture.
	uint32_t mipSide = getTextureMipSize(m_desc.side, level);

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
		*commandBuffer,
		*m_stagingBuffer,
		m_textureImage->getVkImage(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	// Change layout of texture to optimal sampling.
	m_textureImage->changeLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, level, 1, side, 1);

	commandBuffer->submitAndWait();
}

	}
}
