#include "Spark/BitmapTexture.h"
#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.BitmapTexture", 0, BitmapTexture, Bitmap)

BitmapTexture::BitmapTexture(render::ISimpleTexture* texture)
:	Bitmap()
,	m_texture(texture)
{
	m_width = uint32_t(m_texture->getWidth());
	m_height = uint32_t(m_texture->getHeight());
}

	}
}
