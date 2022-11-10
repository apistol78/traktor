/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
