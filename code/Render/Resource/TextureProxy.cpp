/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Resource/TextureProxy.h"

namespace traktor
{
	namespace render
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

ITexture* TextureProxy::resolve()
{
	return m_texture->resolve();
}

	}
}
