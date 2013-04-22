#include "Compress/Lzf/InflateStreamLzf.h"
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Render/ICubeTexture.h"
#include "Render/IVolumeTexture.h"
#include "Render/Resource/TextureFactory.h"
#include "Render/Resource/TextureResource.h"

namespace traktor
{
	namespace render
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureFactory", TextureFactory, resource::IResourceFactory)

TextureFactory::TextureFactory(db::Database* db, IRenderSystem* renderSystem, int32_t skipMips)
:	m_db(db)
,	m_renderSystem(renderSystem)
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

const TypeInfoSet TextureFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TextureResource >());
	return typeSet;
}

const TypeInfoSet TextureFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	type_of< ITexture >().findAllOf(typeSet);
	return typeSet;
}

bool TextureFactory::isCacheable() const
{
	return true;
}

Ref< Object > TextureFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const
{
	Ref< ITexture > texture;

	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< TextureResource > resource = instance->getObject< TextureResource >();
	if (!resource)
		return 0;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	Reader reader(stream);

	uint32_t version;
	reader >> version;
	if (version != 11)
	{
		log::error << L"Unable to read texture, unknown version " << version << Endl;
		return 0;
	}

	int32_t imageWidth, imageHeight, imageDepth, mipCount, texelFormat;
	uint8_t textureType;
	bool sRGB, compressed;

	reader >> imageWidth;
	reader >> imageHeight;
	reader >> imageDepth;
	reader >> mipCount;
	reader >> texelFormat;
	reader >> sRGB;
	reader >> textureType;
	reader >> compressed;

	if (textureType == Tt2D)	// 2D
	{
		int32_t skipMips = (m_skipMips < mipCount) ? m_skipMips : 0;

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

		uint32_t textureDataSize = mipChainSize(desc.format, desc.width, desc.height, desc.mipCount);
		AutoArrayPtr< uint8_t > buffer(new uint8_t [textureDataSize]);

		Ref< IStream > readerStream = stream;
		if (compressed)
			readerStream = new compress::InflateStreamLzf(stream);

		Reader readerData(readerStream);

		uint8_t* data = buffer.ptr();
		for (int i = 0; i < mipCount; ++i)
		{
			int32_t mipWidth = std::max(imageWidth >> i, 1);
			int32_t mipHeight = std::max(imageHeight >> i, 1);
						
			uint32_t mipPitch = getTextureMipPitch((TextureFormat)texelFormat, mipWidth, mipHeight);

			if (i >= skipMips)
			{
				desc.initialData[i - skipMips].data = data;
				desc.initialData[i - skipMips].pitch = getTextureRowPitch(desc.format, mipWidth);

				int32_t nread = readerData.read(data, mipPitch);
				T_ASSERT (nread == mipPitch);

				data += mipPitch;
				T_ASSERT (size_t(data - buffer.ptr()) <= textureDataSize);
			}
			else
			{
				readerStream->seek(
					IStream::SeekCurrent,
					mipPitch
				);
			}
		}

		texture = m_renderSystem->createSimpleTexture(desc);
		if (!texture)
			log::error << L"Unable to create texture, null texture" << Endl;
	}
	else if (textureType == Tt3D)	// 3D
	{
		int32_t skipMips = (m_skipMips < mipCount) ? m_skipMips : 0;

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

		uint32_t sliceDataSize = mipChainSize(desc.format, desc.width, desc.height, desc.mipCount);
		uint32_t textureDataSize = sliceDataSize * desc.depth;
		AutoArrayPtr< uint8_t > buffer(new uint8_t [textureDataSize]);

		Ref< IStream > readerStream = stream;
		if (compressed)
			readerStream = new compress::InflateStreamLzf(stream);

		Reader readerData(readerStream);
		
		// Setup immutable data pointers.
		uint8_t* data = buffer.ptr();
		for (int32_t i = 0; i < mipCount; ++i)
		{
			int32_t mipWidth = std::max(imageWidth >> i, 1);
			int32_t mipHeight = std::max(imageHeight >> i, 1);
						
			uint32_t mipPitch = getTextureMipPitch((TextureFormat)texelFormat, mipWidth, mipHeight);

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
				int32_t mipWidth = std::max(imageWidth >> i, 1);
				int32_t mipHeight = std::max(imageHeight >> i, 1);
						
				uint32_t mipPitch = getTextureMipPitch((TextureFormat)texelFormat, mipWidth, mipHeight);

				if (i >= skipMips)
				{
					int32_t nread = readerData.read(data, mipPitch);
					T_ASSERT (nread == mipPitch);

					data += mipPitch;
					T_ASSERT (size_t(data - buffer.ptr()) <= textureDataSize);
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

		texture = m_renderSystem->createVolumeTexture(desc);
		if (!texture)
			log::error << L"Unable to create texture, null texture" << Endl;
	}
	else if (textureType == TtCube)	// Cube
	{
		if (imageWidth != imageHeight)
		{
			log::error << L"Unable to create texture, width and height must be identical on cube textures" << Endl;
			return 0;
		}

		CubeTextureCreateDesc desc;

		desc.side = imageWidth;
		desc.mipCount = mipCount;
		desc.format = (TextureFormat)texelFormat;
		desc.sRGB = sRGB;
		desc.immutable = true;

		uint32_t textureDataSize = mipChainSize(desc.format, desc.side, desc.side, desc.mipCount);

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
				uint32_t mipPitch = getTextureMipPitch(desc.format, desc.side, desc.side, i);

				desc.initialData[side * mipCount + i].data = data;
				desc.initialData[side * mipCount + i].pitch = getTextureRowPitch(desc.format, desc.side, i);

				int32_t nread = readerData.read(data, mipPitch);
				T_ASSERT (nread == mipPitch);

				data += mipPitch;
				T_ASSERT (size_t(data - buffer[side].ptr()) <= textureDataSize);
			}
		}

		texture = m_renderSystem->createCubeTexture(desc);
		if (!texture)
			log::error << L"Unable to create texture, null texture" << Endl;
	}

	stream->close();
	return texture;
}

	}
}
