#include "Render/TextureFactory.h"
#include "Render/TextureResource.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Render/ICubeTexture.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Zip/InflateStream.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"

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

TextureFactory::TextureFactory(db::Database* db, IRenderSystem* renderSystem)
:	m_db(db)
,	m_renderSystem(renderSystem)
{
}

const TypeSet TextureFactory::getResourceTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< ITexture >());
	return typeSet;
}

bool TextureFactory::isCacheable() const
{
	return true;
}

Object* TextureFactory::create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid)
{
	Ref< ITexture > texture;

	Ref< db::Instance > instance = m_db->getInstance(guid);
	if (!instance)
		return 0;

	Ref< TextureResource > resource = instance->getObject< TextureResource >();
	if (!resource)
		return 0;

	Ref< Stream > stream = instance->readData(L"Data");
	if (!stream)
		return 0;

	Reader reader(stream);

	uint32_t version;
	reader >> version;
	if (version != 4)
	{
		log::error << L"Unable to read texture, unknown version " << version << Endl;
		return 0;
	}

	int32_t imageWidth, imageHeight, mipCount, texelFormat;
	reader >> imageWidth;
	reader >> imageHeight;
	reader >> mipCount;
	reader >> texelFormat;

	bool isCubeMap;
	reader >> isCubeMap;

	bool isCompressed;
	reader >> isCompressed;

	if (!isCubeMap)
	{
		SimpleTextureCreateDesc desc;

		desc.width = imageWidth;
		desc.height = imageHeight;
		desc.mipCount = mipCount;
		desc.format = (TextureFormat)texelFormat;
		desc.immutable = true;

		uint32_t textureDataSize = mipChainSize(desc.format, desc.width, desc.height, desc.mipCount);
		AutoArrayPtr< uint8_t > buffer(new uint8_t [textureDataSize]);

		uint32_t blockSize = getTextureBlockSize(desc.format);
		uint32_t blockDenom = getTextureBlockDenom(desc.format);

		Reader readerData(isCompressed ? gc_new< zip::InflateStream >(stream) : stream);

		uint8_t* data = buffer.ptr();
		for (int i = 0; i < mipCount; ++i)
		{
			int mipWidth = imageWidth >> i;
			int mipHeight = imageHeight >> i;

			uint32_t blockWidth = (mipWidth + blockDenom - 1) / blockDenom;
			uint32_t blockHeight = (mipHeight + blockDenom - 1) / blockDenom;
			uint32_t blockCount = blockWidth * blockHeight;

			desc.initialData[i].data = data;
			desc.initialData[i].pitch = getTextureRowPitch(desc.format, mipWidth);

			int nread = readerData.read(data, blockCount * blockSize, 1);
			T_ASSERT (nread == blockCount * blockSize);

			data += blockCount * blockSize;
			T_ASSERT (size_t(data - buffer.ptr()) <= textureDataSize);
		}

		texture = m_renderSystem->createSimpleTexture(desc);
		if (!texture)
			log::error << L"Unable to create texture, null texture" << Endl;
	}
	else
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
		desc.immutable = true;

		uint32_t textureDataSize = mipChainSize(desc.format, desc.side, desc.side, desc.mipCount);

		AutoArrayPtr< uint8_t > buffer[6];

		Reader readerData(isCompressed ? gc_new< zip::InflateStream >(stream) : stream);
		
		for (int side = 0; side < 6; ++side)
		{
			buffer[side] = new uint8_t [textureDataSize];

			uint8_t* data = buffer[side].ptr();
			for (int i = 0; i < mipCount; ++i)
			{
				int mipSide = imageWidth >> i;

				desc.initialData[side * mipCount + i].data = data;
				desc.initialData[side * mipCount + i].pitch = mipSide * 4;

				readerData.read(data, mipSide * mipSide, 4);

				data += mipSide * mipSide * 4;
			}
		}

		texture = m_renderSystem->createCubeTexture(desc);
		if (!texture)
			log::error << L"Unable to create texture, null texture" << Endl;
	}

	return texture;
}

	}
}
