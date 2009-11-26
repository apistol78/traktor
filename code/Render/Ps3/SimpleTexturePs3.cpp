#include <cstring>
#include "Render/Ps3/SimpleTexturePs3.h"
#include "Render/Ps3/LocalMemoryAllocator.h"
#include "Render/Ps3/TypesPs3.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

bool isPowerOf2(int value)
{
	return bool((value & -value) == value);
}

uint32_t calculateMipPitch(uint32_t pitch, int mipLevel)
{
	//return std::max< uint32_t >(pitch >> mipLevel, 64);
	return pitch >> mipLevel;
}

uint32_t calculateMipSize(uint32_t pitch, uint32_t height, int mipLevel)
{
	return calculateMipPitch(pitch, mipLevel) * (height >> mipLevel);
}

uint32_t calculateTextureSize(uint32_t pitch, uint32_t height, int mipCount)
{
	uint32_t textureSize = 0;
	for (int i = 0; i < mipCount; ++i)
		textureSize += calculateMipSize(pitch, height, i);
	return textureSize;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTexturePs3", SimpleTexturePs3, ISimpleTexture)

SimpleTexturePs3::SimpleTexturePs3()
:	m_data(0)
,	m_offset(0)
{
	std::memset(&m_texture, 0, sizeof(m_texture));
}

SimpleTexturePs3::~SimpleTexturePs3()
{
	destroy();
}

bool SimpleTexturePs3::create(const SimpleTextureCreateDesc& desc)
{
	int byteSize = 0;
	if (!getGcmTextureInfo(desc.format, byteSize, m_texture.format))
	{
		log::error << L"Unable to create texture; unsupported format" << Endl;
		return false;
	}

	bool linear = false;
	if (desc.immutable)
	{
		if (!isPowerOf2(desc.width) || !isPowerOf2(desc.height))
		{
			log::warning << "Non-power of 2 texture, cannot use swizzled texture format" << Endl;
			linear = true;
		}
	}
	else
		linear = true;

	if (linear)
		m_texture.format |= CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR;
	else
		m_texture.format |= CELL_GCM_TEXTURE_SZ | CELL_GCM_TEXTURE_NR;

	uint32_t texturePitch = desc.width * byteSize;

	m_texture.mipmap = desc.mipCount;
	m_texture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
	m_texture.cubemap = 0;
	m_texture.remap =
		CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
		CELL_GCM_TEXTURE_REMAP_FROM_G << 6 |
		CELL_GCM_TEXTURE_REMAP_FROM_R << 4 |
		CELL_GCM_TEXTURE_REMAP_FROM_A << 2 |
		CELL_GCM_TEXTURE_REMAP_FROM_B;
	m_texture.width = desc.width;
	m_texture.height = desc.height;
	m_texture.depth = 1;
	m_texture.location = CELL_GCM_LOCATION_LOCAL;
	if (linear)
		m_texture.pitch = texturePitch;
	m_texture.offset = 0;

	uint32_t textureSize = calculateTextureSize(
		texturePitch,
		m_texture.height,
		m_texture.mipmap
	);

	m_data = LocalMemoryAllocator::getInstance().allocAlign(textureSize, 128);
	if (cellGcmAddressToOffset(m_data, &m_texture.offset) != CELL_OK)
		return false;

	if (desc.immutable)
	{
		uint32_t offset = 0;
		for (int i = 0; i < m_texture.mipmap; ++i)
		{
			uint32_t size = calculateMipSize(texturePitch, m_texture.height, i);

			const uint8_t* src = static_cast< const uint8_t* >(desc.initialData[i].data);
			uint8_t* dest = static_cast< uint8_t* >(m_data) + offset;

			if (linear)
			{
				std::memcpy(dest, src, size);
			}
			else
			{
				int32_t mipWidth = std::max< int32_t >(m_texture.width >> i, 1);
				int32_t mipHeight = std::max< int32_t >(m_texture.height >> i, 1);

				cellUtilConvertLinearToSwizzle(
					dest,
					src,
					mipWidth,
					mipHeight,
					byteSize
				);
			}

			offset += size;
		}
	}

	return true;
}

void SimpleTexturePs3::destroy()
{
	if (m_data)
	{
		LocalMemoryAllocator::getInstance().free(m_data);

		m_data = 0;
		m_offset = 0;
	}
}

int SimpleTexturePs3::getWidth() const
{
	return m_texture.width;
}

int SimpleTexturePs3::getHeight() const
{
	return m_texture.height;
}

int SimpleTexturePs3::getDepth() const
{
	return m_texture.depth;
}

bool SimpleTexturePs3::lock(int level, Lock& lock)
{
	if (m_texture.pitch != 0 && level == 0)
	{
		uint8_t* ptr = static_cast< uint8_t* >(m_data);
		lock.bits = ptr;
		lock.pitch = m_texture.pitch;
		return true;
	}
	else
		return false;
}

void SimpleTexturePs3::unlock(int level)
{
}

void SimpleTexturePs3::bind(int stage, const SamplerState& samplerState)
{
	cellGcmSetTextureControl(
		gCellGcmCurrentContext,
		stage,
		CELL_GCM_TRUE,
		0,
		m_texture.mipmap << 8,
		CELL_GCM_TEXTURE_MAX_ANISO_1
	);

	cellGcmSetTextureFilter(
		gCellGcmCurrentContext,
		stage,
		0,
		samplerState.minFilter,
		samplerState.magFilter,
		CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX
	);

	cellGcmSetTextureAddress(
		gCellGcmCurrentContext,
		stage,
		samplerState.wrapU,
		samplerState.wrapV,
		samplerState.wrapW,
		CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,
		CELL_GCM_TEXTURE_ZFUNC_NEVER,
		0
	);

	cellGcmSetTexture(
		gCellGcmCurrentContext,
		stage,
		&m_texture
	);
}

	}
}
