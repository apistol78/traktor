#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/ITexture.h"
#include "Render/Image2/ImageTexture.h"
#include "Render/Image2/ImageTextureData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageTextureData", 0, ImageTextureData, ISerializable)

Ref< const ImageTexture > ImageTextureData::createInstance(resource::IResourceManager* resourceManager) const
{
    resource::Proxy< ITexture > texture;
    if (!resourceManager->bind(m_texture, texture))
        return nullptr;

	handle_t textureId = getParameterHandle(m_textureId);

    return new ImageTexture(
		textureId,
        texture
    );
}

void ImageTextureData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"textureId", m_textureId);
    s >> resource::Member< render::ITexture >(L"texture", m_texture);
}

    }
}
