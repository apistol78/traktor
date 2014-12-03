#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/Endian.h"
#include "Render/Ps3/MemoryHeap.h"
#include "Render/Ps3/MemoryHeapObject.h"
#include "Render/Ps3/VolumeTexturePs3.h"
#include "Render/Ps3/StateCachePs3.h"
#include "Render/Ps3/TypesPs3.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTexturePs3", VolumeTexturePs3, IVolumeTexture)

VolumeTexturePs3::VolumeTexturePs3(int32_t& counter)
:	m_data(0)
,	m_counter(counter)
{
	std::memset(&m_texture, 0, sizeof(m_texture));
	++m_counter;
}

VolumeTexturePs3::~VolumeTexturePs3()
{
	destroy();
}

bool VolumeTexturePs3::create(MemoryHeap* memoryHeap, const VolumeTextureCreateDesc& desc)
{
	if (!getGcmTextureInfo(desc.format, m_texture.format))
	{
		log::error << L"Unable to create texture; unsupported format" << Endl;
		return false;
	}

	const bool linear = false;

	if (linear)
		m_texture.format |= CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR;
	else
		m_texture.format |= CELL_GCM_TEXTURE_SZ | CELL_GCM_TEXTURE_NR;

	bool dxtn = (desc.format >= TfDXT1 && desc.format <= TfDXT5);
	uint32_t blockSize = getTextureBlockSize(desc.format);
	uint32_t blockDenom = getTextureBlockDenom(desc.format);
	uint32_t texturePitch = getTextureRowPitch(desc.format, desc.width);

	m_texture.mipmap = desc.mipCount;
	m_texture.dimension = CELL_GCM_TEXTURE_DIMENSION_3;
	m_texture.cubemap = CELL_GCM_FALSE;

	if (desc.format >= TfDXT1 && desc.format <= TfDXT5)
	{
		// GBAR (LE ARGB8)
		m_texture.remap =
			CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
			CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
			CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
			CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
			CELL_GCM_TEXTURE_REMAP_FROM_A;
	}
	else if (desc.format >= TfR16G16B16A16F && desc.format <= TfR11G11B10F)
	{
		// ARGB (Cannot remap FP textures).
		m_texture.remap =
			CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
			CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
			CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
			CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
			CELL_GCM_TEXTURE_REMAP_FROM_A;
	}
	else
	{
		// BARG (LE RGBA8)
		m_texture.remap =
			CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
			CELL_GCM_TEXTURE_REMAP_FROM_G << 6 |
			CELL_GCM_TEXTURE_REMAP_FROM_R << 4 |
			CELL_GCM_TEXTURE_REMAP_FROM_A << 2 |
			CELL_GCM_TEXTURE_REMAP_FROM_B;
	}

	m_texture.width = desc.width;
	m_texture.height = desc.height;
	m_texture.depth = desc.depth;
	m_texture.location = CELL_GCM_LOCATION_LOCAL;
	m_texture.pitch = texturePitch;
	m_texture.offset = 0;

	uint32_t textureSize = getTextureSize(
		desc.format,
		desc.width,
		desc.height,
		desc.mipCount
	);

	textureSize *= desc.depth;

	m_data = memoryHeap->alloc(textureSize, 128, false);
	if (!m_data)
		return false;

	if (desc.immutable)
	{
		uint32_t offset = 0;
		for (uint32_t slice = 0; slice < desc.depth; ++slice)
		{
			for (uint32_t i = 0; i < desc.mipCount; ++i)
			{
				uint32_t mipWidth = getTextureMipSize(desc.width, i);
				uint32_t mipHeight = getTextureMipSize(desc.height, i);
				uint32_t mipSize = getTextureMipPitch(desc.format, mipWidth, mipHeight);
				uint32_t mipPitch = getTextureRowPitch(desc.format, desc.width, i);

				const uint8_t* src = static_cast< const uint8_t* >(desc.initialData[i].data) + desc.initialData[i].slicePitch * slice;
				uint8_t* dest = static_cast< uint8_t* >(m_data->getPointer()) + offset;

				if (dxtn || linear)
				{
					uint32_t blockRows = std::max< uint32_t >(mipWidth / blockDenom, 1);
					for (uint32_t y = 0; y < blockRows; ++y)
					{
						std::memcpy(dest, src, desc.initialData[i].pitch);
						src += desc.initialData[i].pitch;
						dest += mipPitch;
					}
				}
				else
				{
					cellUtilConvertLinearToSwizzle(
						dest,
						src,
						mipWidth,
						mipHeight,
						getTextureBlockSize(desc.format)
					);
				}

				offset += mipSize;
			}
		}
	}

	return true;
}

void VolumeTexturePs3::destroy()
{
	if (m_data)
	{
		m_data->free(); m_data = 0;
		--m_counter;
	}
}

ITexture* VolumeTexturePs3::resolve()
{
	return this;
}

int VolumeTexturePs3::getWidth() const
{
	return m_texture.width;
}

int VolumeTexturePs3::getHeight() const
{
	return m_texture.height;
}

int VolumeTexturePs3::getDepth() const
{
	return m_texture.depth;
}

void VolumeTexturePs3::bind(StateCachePs3& stateCache, int stage, const SamplerStateGCM& samplerState)
{
	if (m_data)
	{
		m_texture.offset = m_data->getOffset();
		stateCache.setSamplerState(stage, samplerState);
		stateCache.setSamplerTexture(
			stage,
			&m_texture,
			m_texture.mipmap << 8,
			m_texture.mipmap > 1 ? CELL_GCM_TEXTURE_MAX_ANISO_2 : CELL_GCM_TEXTURE_MAX_ANISO_1
		);
	}
}

	}
}
