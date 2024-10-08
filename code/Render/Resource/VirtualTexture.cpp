/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Compress/Lzf/InflateStreamLzf.h"
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/ThreadManager.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/VirtualTexture.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VirtualTexture", VirtualTexture, ITexture)

VirtualTexture::VirtualTexture(IRenderSystem* renderSystem, IStream* stream, ITexture* lowQualityMipTexture)
:	m_renderSystem(renderSystem)
,	m_stream(stream)
,	m_lowQualityMipTexture(lowQualityMipTexture)
,	m_highQualityLoadedOrPending(false)
{
	stream->seek(IStream::SeekSet, 0);

	Reader reader(stream);

	uint32_t version;
	reader >> version;
	T_FATAL_ASSERT(version == 12);

	int32_t imageDepth;
	uint8_t textureType;
	bool system;

	reader >> m_imageWidth;
	reader >> m_imageHeight;
	reader >> imageDepth;
	reader >> m_mipCount;
	reader >> m_texelFormat;
	reader >> m_sRGB;
	reader >> textureType;
	reader >> m_compressed;
	reader >> system;

	m_textureDataOffset = stream->tell();
}

void VirtualTexture::destroy()
{
	safeDestroy(m_lowQualityMipTexture);
	safeDestroy(m_highQualityMipTexture);
}

ITexture::Size VirtualTexture::getSize() const
{
	return { m_imageWidth, m_imageHeight, 1, m_mipCount };
}

bool VirtualTexture::lock(int32_t side, int32_t level, Lock& lock)
{
	return false;
}

void VirtualTexture::unlock(int32_t side, int32_t level)
{
}

ITexture* VirtualTexture::resolve()
{
	if (!m_highQualityLoadedOrPending)
	{
		m_highQualityLoadedOrPending = true;
		JobManager::getInstance().add([this]() {
			loadHighQuality();
		});
	}
	return m_highQualityMipTexture != nullptr ? m_highQualityMipTexture : m_lowQualityMipTexture;
}

bool VirtualTexture::loadHighQuality()
{
	int32_t skipMips = 0;

	// Do not skip mips on already small enough textures.
	if (m_imageWidth <= 16 || m_imageHeight <= 16)
		skipMips = 0;

	SimpleTextureCreateDesc desc;
	desc.width = m_imageWidth >> skipMips;
	desc.height = m_imageHeight >> skipMips;
	desc.mipCount = m_mipCount - skipMips;
	desc.format = (TextureFormat)m_texelFormat;
	desc.sRGB = m_sRGB;
	desc.immutable = true;

	const uint32_t textureDataSize = getTextureSize(desc.format, desc.width, desc.height, desc.mipCount);
	AutoArrayPtr< uint8_t > buffer(new uint8_t [textureDataSize]);

	m_stream->seek(IStream::SeekSet, m_textureDataOffset);

	Ref< IStream > readerStream = m_stream;
	if (m_compressed)
		readerStream = new compress::InflateStreamLzf(m_stream);

	Reader readerData(readerStream);

	uint8_t* data = buffer.ptr();
	for (int i = 0; i < m_mipCount; ++i)
	{
		const int32_t mipWidth = std::max(m_imageWidth >> i, 1);
		const int32_t mipHeight = std::max(m_imageHeight >> i, 1);
		const uint32_t mipPitch = getTextureMipPitch((TextureFormat)m_texelFormat, mipWidth, mipHeight);

		if (i >= skipMips)
		{
			desc.initialData[i - skipMips].data = data;
			desc.initialData[i - skipMips].pitch = getTextureRowPitch(desc.format, mipWidth);

			const int64_t nread = readerData.read(data, mipPitch);
			if (nread != mipPitch)
			{
				log::error << L"Unable to read texture; not enough data in stream." << Endl;
				return false;
			}

			data += mipPitch;
			T_ASSERT(size_t(data - buffer.ptr()) <= textureDataSize);
		}
		else
		{
			readerStream->seek(
				IStream::SeekCurrent,
				mipPitch
			);
		}
	}

	m_highQualityMipTexture = m_renderSystem->createSimpleTexture(desc, T_FILE_LINE_W);
	if (!m_highQualityMipTexture)
		log::error << L"Unable to create 2D texture resource; failed to create renderable texture." << Endl;

	return true;
}

}
