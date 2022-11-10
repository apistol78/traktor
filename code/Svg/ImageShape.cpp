/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Svg/ImageShape.h"

namespace traktor
{
	namespace svg
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.ImageShape", ImageShape, Shape)

ImageShape::ImageShape(const Vector2& position, const Vector2& size, const drawing::Image* image)
:	m_position(position)
,   m_size(size)
,   m_image(image)
{
}

const Vector2& ImageShape::getPosition() const
{
	return m_position;
}

const Vector2& ImageShape::getSize() const
{
	return m_size;
}

const drawing::Image* ImageShape::getImage() const
{
	return m_image;
}

	}
}
