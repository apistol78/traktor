/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Image2/ImageTexture.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageTexture", ImageTexture, Object)

ImageTexture::ImageTexture(img_handle_t textureId, const resource::Proxy< ITexture >& texture)
:   m_textureId(textureId)
,   m_texture(texture)
{
}

img_handle_t ImageTexture::getTextureId() const
{
	return m_textureId;
}

const resource::Proxy< ITexture >& ImageTexture::getTexture() const
{
    return m_texture;
}

}
