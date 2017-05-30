/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/BitmapTexture.h"
#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.BitmapTexture", 0, BitmapTexture, Bitmap)

BitmapTexture::BitmapTexture()
:	Bitmap()
{
}

BitmapTexture::BitmapTexture(render::ISimpleTexture* texture)
:	Bitmap()
,	m_texture(texture)
{
	m_width = uint32_t(m_texture->getWidth());
	m_height = uint32_t(m_texture->getHeight());
}

	}
}
