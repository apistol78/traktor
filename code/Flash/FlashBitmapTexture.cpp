#include "Flash/FlashBitmapTexture.h"
#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashBitmapTexture", 0, FlashBitmapTexture, FlashBitmap)

FlashBitmapTexture::FlashBitmapTexture()
:	FlashBitmap()
{
}

FlashBitmapTexture::FlashBitmapTexture(render::ISimpleTexture* texture)
:	FlashBitmap()
,	m_texture(texture)
{
	m_width = uint32_t(m_texture->getWidth());
	m_height = uint32_t(m_texture->getHeight());
}

	}
}
