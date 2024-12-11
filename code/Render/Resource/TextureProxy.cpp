/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Resource/TextureProxy.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureProxy", TextureProxy, ITexture)

TextureProxy::TextureProxy(const resource::Proxy< ITexture >& texture)
:	m_texture(texture)
{
}

void TextureProxy::destroy()
{
	m_texture.clear();
}

ITexture::Size TextureProxy::getSize() const
{
	return m_texture->getSize();
}

int32_t TextureProxy::getBindlessIndex() const
{
	return m_texture->getBindlessIndex();
}

bool TextureProxy::lock(int32_t side, int32_t level, Lock& lock)
{
	return m_texture->lock(side, level, lock);
}

void TextureProxy::unlock(int32_t side, int32_t level)
{
	m_texture->unlock(side, level);
}

ITexture* TextureProxy::resolve()
{
	return m_texture->resolve();
}

}
