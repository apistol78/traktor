/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Compress/Lzf/InflateStreamLzf.h"
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/ObjectStore.h"
#include "Database/Instance.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/TextureFactory.h"
#include "Render/Resource/TextureResource.h"
#include "Resource/IResourceManager.h"

#undef min
#undef max

namespace traktor::render
{
	namespace
	{

uint32_t mipChainSize(TextureFormat format, int width, int height, int mipCount)
{
	uint32_t totalSize = 0;
	for (int i = 0; i < mipCount; ++i)
		totalSize += getTextureMipPitch(format, width, height, i);
	return totalSize;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureFactory", 0, TextureFactory, resource::IResourceFactory)

TextureFactory::TextureFactory(IRenderSystem* renderSystem, int32_t skipMips)
:	m_renderSystem(renderSystem)
,	m_skipMips(skipMips)
{
}

void TextureFactory::setSkipMips(int32_t skipMips)
{
	m_skipMips = skipMips;
}

int32_t TextureFactory::getSkipMips() const
{
	return m_skipMips;
}

bool TextureFactory::initialize(const ObjectStore& objectStore)
{
	m_renderSystem = objectStore.get< IRenderSystem >();
	return true;
}

const TypeInfoSet TextureFactory::getResourceTypes() const
{
	return makeTypeInfoSet< TextureResource >();
}

const TypeInfoSet TextureFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< ITexture >();
}

bool TextureFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > TextureFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< ITexture > texture;

	Ref< const TextureResource > resource = instance->getObject< TextureResource >();
	if (!resource)
		return nullptr;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
	{
		log::error << L"Unable to read texture; no data associated with instance." << Endl;
		return nullptr;
	}

	Reader reader(stream);

	uint32_t version;
	reader >> version;
	if (version != 12)
	{
		log::error << L"Unable to read texture; unknown version " << version << L"." << Endl;
		return nullptr;
	}

	int32_t imageWidth, imageHeight, imageDepth, mipCount, texelFormat;
	uint8_t textureType;
	bool sRGB, compressed, system;

	reader >> imageWidth;
	reader >> imageHeight;
	reader >> imageDepth;
	reader >> mipCount;
	reader >> texelFormat;
	reader >> sRGB;
	reader >> textureType;
	reader >> compressed;
	reader >> system;

	if (textureType == Tt2D)	// 2D
	{
		int32_t skipMips = (!system && m_skipMips < mipCount) ? m_skipMips : 0;

		// Do not skip mips on already small enough textures.
		if (imageWidth <= 16 || imageHeight <= 16)
			skipMips = 0;

		SimpleTextureCreateDesc desc;
		desc.width = imageWidth >> skipMips;
		desc.height = imageHeight >> skipMips;
		desc.mipCount = mipCount - skipMips;
		desc.format = (TextureFormat)texelFormat;
		desc.sRGB = sRGB;
		desc.immutable = true;

		const uint32_t textureDataSize = mipChainSize(desc.format, desc.width, desc.height, desc.mipCount);
		AutoArrayPtr< uint8_t > buffer(new uint8_t [textureDataSize]);

		Ref< IStream > readerStream = stream;
		if (compressed)
			readerStream = new compress::InflateStreamLzf(stream);

		Reader readerData(readerStream);

		uint8_t* data = buffer.ptr();
		for (int i = 0; i < mipCount; ++i)
		{
			const int32_t mipWidth = std::max(imageWidth >> i, 1);
			const int32_t mipHeight = std::max(imageHeight >> i, 1);
			const uint32_t mipPitch = getTextureMipPitch((TextureFormat)texelFormat, mipWidth, mipHeight);

			if (i >= skipMips)
			{
				desc.initialData[i - skipMips].data = data;
				desc.initialData[i - skipMips].pitch = getTextureRowPitch(desc.format, mipWidth);

				int64_t nread = readerData.read(data, mipPitch);
				if (nread != mipPitch)
				{
					log::error << L"Unable to read texture; not enough data in stream." << Endl;
					return nullptr;
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

		texture = m_renderSystem->createSimpleTexture(desc, instance->getName().c_str());
		if (!texture)
			log::error << L"Unable to create 2D texture resource; failed to create renderable texture." << Endl;
	}
	else if (textureType == Tt3D)	// 3D
	{
		int32_t skipMips = (!system && m_skipMips < mipCount) ? m_skipMips : 0;

		// Do not skip mips on already small enough textures.
		if (imageWidth <= 16 || imageHeight <= 16)
			skipMips = 0;

		VolumeTextureCreateDesc desc;

		desc.width = imageWidth >> skipMips;
		desc.height = imageHeight >> skipMips;
		desc.depth = imageDepth >> skipMips;
		desc.mipCount = mipCount - skipMips;
		desc.format = (TextureFormat)texelFormat;
		desc.sRGB = sRGB;
		desc.immutable = true;

		const uint32_t sliceDataSize = mipChainSize(desc.format, desc.width, desc.height, desc.mipCount);
		const uint32_t textureDataSize = sliceDataSize * desc.depth;
		AutoArrayPtr< uint8_t > buffer(new uint8_t [textureDataSize]);

		Ref< IStream > readerStream = stream;
		if (compressed)
			readerStream = new compress::InflateStreamLzf(stream);

		Reader readerData(readerStream);

		// Setup immutable data pointers.
		uint8_t* data = buffer.ptr();
		for (int32_t i = 0; i < mipCount; ++i)
		{
			const int32_t mipWidth = std::max(imageWidth >> i, 1);
			const int32_t mipHeight = std::max(imageHeight >> i, 1);
			const uint32_t mipPitch = getTextureMipPitch((TextureFormat)texelFormat, mipWidth, mipHeight);

			if (i >= skipMips)
			{
				desc.initialData[i - skipMips].data = data;
				desc.initialData[i - skipMips].pitch = getTextureRowPitch(desc.format, mipWidth);
				desc.initialData[i - skipMips].slicePitch = sliceDataSize;
			}

			data += mipPitch;
		}

		// Read each slice.
		for (int32_t slice = 0; slice < desc.depth; ++slice)
		{
			data = buffer.ptr() + slice * sliceDataSize;
			for (int32_t i = 0; i < mipCount; ++i)
			{
				const int32_t mipWidth = std::max(imageWidth >> i, 1);
				const int32_t mipHeight = std::max(imageHeight >> i, 1);
				const uint32_t mipPitch = getTextureMipPitch((TextureFormat)texelFormat, mipWidth, mipHeight);

				if (i >= skipMips)
				{
					const int64_t nread = readerData.read(data, mipPitch);
					if (nread != mipPitch)
					{
						log::error << L"Unable to read texture; not enough data in stream." << Endl;
						return nullptr;
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
		}

		texture = m_renderSystem->createVolumeTexture(desc, instance->getName().c_str());
		if (!texture)
			log::error << L"Unable to create 3D texture resource; failed to create renderable texture." << Endl;
	}
	else if (textureType == TtCube)	// Cube
	{
		if (imageWidth != imageHeight)
		{
			log::error << L"Unable to create CUBE texture resource; width and height must be identical on cube textures." << Endl;
			return nullptr;
		}

		CubeTextureCreateDesc desc;

		desc.side = imageWidth;
		desc.mipCount = mipCount;
		desc.format = (TextureFormat)texelFormat;
		desc.sRGB = sRGB;
		desc.immutable = true;

		const uint32_t textureDataSize = mipChainSize(desc.format, desc.side, desc.side, desc.mipCount);

		AutoArrayPtr< uint8_t > buffer[6];

		Ref< IStream > readerStream = stream;
		if (compressed)
			readerStream = new compress::InflateStreamLzf(stream);

		Reader readerData(readerStream);

		for (int32_t side = 0; side < 6; ++side)
		{
			buffer[side].reset(new uint8_t [textureDataSize]);

			uint8_t* data = buffer[side].ptr();
			for (int32_t i = 0; i < mipCount; ++i)
			{
				const uint32_t mipPitch = getTextureMipPitch(desc.format, desc.side, desc.side, i);

				desc.initialData[side * mipCount + i].data = data;
				desc.initialData[side * mipCount + i].pitch = getTextureRowPitch(desc.format, desc.side, i);

				const int64_t nread = readerData.read(data, mipPitch);
				if (nread != mipPitch)
				{
					log::error << L"Unable to read texture; not enough data in stream." << Endl;
					return nullptr;
				}

				data += mipPitch;
				T_ASSERT(size_t(data - buffer[side].ptr()) <= textureDataSize);
			}
		}

		texture = m_renderSystem->createCubeTexture(desc, instance->getName().c_str());
		if (!texture)
			log::error << L"Unable to create CUBE texture resource; failed to create renderable texture." << Endl;
	}

	stream->close();
	return texture;
}

}
