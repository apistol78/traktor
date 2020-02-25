#include "Render/Image2/ImageTexture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageTexture", ImageTexture, Object)

ImageTexture::ImageTexture(handle_t textureId, const resource::Proxy< ITexture >& texture)
:   m_textureId(textureId)
,   m_texture(texture)
{
}

handle_t ImageTexture::getTextureId() const
{
	return m_textureId;
}

const resource::Proxy< ITexture >& ImageTexture::getTexture() const
{
    return m_texture;
}

	}
}
