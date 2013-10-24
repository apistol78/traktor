#include <squish.h>
#include "Render/Sw/CubeTextureSw.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubeTextureSw", CubeTextureSw, ICubeTexture)

CubeTextureSw::CubeTextureSw()
:	m_side(0)
,	m_data(0)
{
}

CubeTextureSw::~CubeTextureSw()
{
	destroy();
}

bool CubeTextureSw::create(const CubeTextureCreateDesc& desc)
{
	m_side = desc.side;
	m_format = desc.format;

	m_data = new uint32_t [m_side * m_side * 6];
	T_ASSERT (m_data);

	// Copy immutable data into texture.
	if (desc.immutable)
	{
		if (desc.format < TfDXT1)
		{
			const uint8_t* src = static_cast< const uint8_t* >(desc.initialData[0].data);
			uint32_t* dst = m_data;

			for (uint32_t i = 0; i < uint32_t(desc.side * desc.side); ++i)
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
		else	// Compressed texture.
		{
			squish::DecompressImage(
				reinterpret_cast< squish::u8* >(m_data),
				desc.side,
				desc.side,
				desc.initialData[0].data,
				desc.format == TfDXT1 ? squish::kDxt1 : squish::kDxt3
			);
		}
	}

	return true;
}

void CubeTextureSw::destroy()
{
	delete[] m_data;
	m_data = 0;
}

ITexture* CubeTextureSw::resolve()
{
	return this;
}

int CubeTextureSw::getWidth() const
{
	return m_side;
}

int CubeTextureSw::getHeight() const
{
	return m_side;
}

int CubeTextureSw::getDepth() const
{
	return m_side;
}

bool CubeTextureSw::lock(int side, int level, Lock& lock)
{
	return false;
}

void CubeTextureSw::unlock(int side, int level)
{
}

	}
}
