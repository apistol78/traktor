/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Drawing/Image.h"
#include "Terrain/Editor/ImageFallOff.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.ImageFallOff", ImageFallOff, IFallOff)

ImageFallOff::ImageFallOff(const drawing::Image* image)
:	m_image(image)
{
}

float ImageFallOff::evaluate(float x, float y) const
{
	const int32_t ix = int32_t((x * 0.5f + 0.5f) * (m_image->getWidth() - 1));
	const int32_t iy = int32_t((y * 0.5f + 0.5f) * (m_image->getHeight() - 1));

	Color4f c;
	if (!m_image->getPixel(ix, iy, c))
		return 0.0f;

	return c.getRed();
}

}
