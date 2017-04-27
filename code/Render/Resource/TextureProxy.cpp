/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
