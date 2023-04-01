/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Spark/BitmapImage.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.BitmapImage", 0, BitmapImage, Bitmap)

BitmapImage::BitmapImage(const drawing::Image* image)
:	Bitmap()
{
	m_image = image->clone();
#if defined(T_LITTLE_ENDIAN)
	if (m_image->getPixelFormat() != drawing::PixelFormat::getA8B8G8R8())
		m_image->convert(drawing::PixelFormat::getA8B8G8R8());
#else
	if (m_image->getPixelFormat() != drawing::PixelFormat::getR8G8B8A8())
		m_image->convert(drawing::PixelFormat::getR8G8B8A8());
#endif

	if (image->getPixelFormat().getAlphaBits() <= 0)
		m_image->clearAlpha(1.0f);

	m_width = uint32_t(m_image->getWidth());
	m_height = uint32_t(m_image->getHeight());
}

const void* BitmapImage::getBits() const
{
	return m_image->getData();
}

void BitmapImage::serialize(ISerializer& s)
{
	Bitmap::serialize(s);
	s >> MemberRef< drawing::Image >(L"image", m_image);
}

	}
}
