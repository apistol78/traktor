/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/BitmapTexture.h"
#include "Render/ITexture.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.BitmapTexture", 0, BitmapTexture, Bitmap)

BitmapTexture::BitmapTexture(render::ITexture* texture)
:	Bitmap()
,	m_texture(texture)
{
	const auto sz = m_texture->getSize();
	m_width = (uint32_t)sz.x;
	m_height = (uint32_t)sz.y;
}

	}
}
