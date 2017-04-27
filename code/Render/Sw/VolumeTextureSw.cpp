/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <squish.h>
#include "Render/Sw/VolumeTextureSw.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureSw", VolumeTextureSw, IVolumeTexture)

VolumeTextureSw::VolumeTextureSw()
:	m_width(0)
,	m_height(0)
,	m_depth(0)
,	m_data(0)
{
}

VolumeTextureSw::~VolumeTextureSw()
{
	destroy();
}

bool VolumeTextureSw::create(const VolumeTextureCreateDesc& desc)
{
	m_width = desc.width;
	m_height = desc.height;
	m_depth = desc.depth;
	m_format = desc.format;

	m_data = new uint32_t [m_width * m_height * m_depth];
	T_ASSERT (m_data);

	// Copy immutable data into texture.
	if (desc.immutable)
	{
		if (desc.format < TfDXT1)
		{
			const uint8_t* src = static_cast< const uint8_t* >(desc.initialData[0].data);
			for (uint32_t slice = 0; slice < m_depth; ++slice)
			{
				uint32_t* dst = m_data + (m_width * m_height) * slice;
				for (uint32_t i = 0; i < uint32_t(desc.width * desc.height); ++i)
				{
					switch (desc.format)
					{
					case TfR8:
						*dst = (src[0] << 24) | (src[0] << 16) | (src[0] << 8) | src[0];
						break;

					case TfR8G8B8A8:
						*dst = (src[3] << 24) | (src[0] << 16) | (src[1] << 8) | src[2];
						break;

					case TfR16G16B16A16F:
					case TfR32G32B32A32F:
					case TfR16G16F:
					case TfR32G32F:
					case TfR16F:
					case TfR32F:
						T_BREAKPOINT;
						break;
					}

					dst += 1;
					src += getTextureBlockSize(desc.format);
				}
			}
		}
		else	// Compressed texture.
			return false;
	}

	return true;
}

void VolumeTextureSw::destroy()
{
	delete[] m_data;
	m_data = 0;
}

ITexture* VolumeTextureSw::resolve()
{
	return this;
}

int VolumeTextureSw::getWidth() const
{
	return m_width;
}

int VolumeTextureSw::getHeight() const
{
	return m_height;
}

int VolumeTextureSw::getDepth() const
{
	return m_depth;
}

	}
}
