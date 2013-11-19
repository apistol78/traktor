#include <squish.h>
#include "Render/Sw/SimpleTextureSw.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureSw", SimpleTextureSw, ISimpleTexture)

SimpleTextureSw::SimpleTextureSw()
:	m_width(0)
,	m_height(0)
,	m_data(0)
,	m_lock(0)
{
}

SimpleTextureSw::~SimpleTextureSw()
{
	destroy();
}

bool SimpleTextureSw::create(const SimpleTextureCreateDesc& desc)
{
	m_width = desc.width;
	m_height = desc.height;
	m_format = desc.format;

	m_data = new uint32_t [m_width * m_height];
	T_ASSERT (m_data);

	// Copy immutable data into texture.
	if (desc.immutable)
	{
		if (desc.format < TfDXT1)
		{
			const uint8_t* src = static_cast< const uint8_t* >(desc.initialData[0].data);
			uint32_t* dst = m_data;

			for (uint32_t i = 0; i < uint32_t(desc.width * desc.height); ++i)
			{
				switch (desc.format)
				{
				case TfR8:
					*dst = (src[0] << 24) | (src[0] << 16) | (src[0] << 8) | src[0];
					break;

				case TfR8G8B8A8:
					*dst = (src[3] << 24) | (src[2] << 16) | (src[1] << 8) | src[0];
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
				desc.width,
				desc.height,
				desc.initialData[0].data,
				desc.format == TfDXT1 ? squish::kDxt1 : squish::kDxt3
			);
		}
	}

	return true;
}

void SimpleTextureSw::destroy()
{
	T_ASSERT_M (!m_lock, L"Texture still locked");
	delete[] m_data;
	m_data = 0;
}

ITexture* SimpleTextureSw::resolve()
{
	return this;
}

int SimpleTextureSw::getWidth() const
{
	return m_width;
}

int SimpleTextureSw::getHeight() const
{
	return m_height;
}

bool SimpleTextureSw::lock(int level, Lock& lock)
{
	if (m_lock)
		return false;

	m_lock = new uint8_t [m_width * m_height * getTextureBlockSize(m_format)];
	
	lock.pitch = m_width * getTextureBlockSize(m_format);
	lock.bits = m_lock;

	return true;
}

void SimpleTextureSw::unlock(int level)
{
	if (!m_lock)
		return;

	if (level == 0)
	{
		if (getTextureBlockSize(m_format) == 4)
			memcpy(m_data, m_lock, m_width * m_height * sizeof(uint32_t));
		else
			T_BREAKPOINT;
	}

	delete[] m_lock;
	m_lock = 0;
}

	}
}
