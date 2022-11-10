/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/Vector2.h"
#include "Drawing/Image.h"
#include "Shape/Editor/Bake/IblProbe.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

Vector2 toEquirectangular(const Vector4& direction)
{
	float theta = std::acos(direction.y());
	float phi = std::atan2(-direction.x(), -direction.z()) + PI;
	return Vector2(phi, theta);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.IblProbe", IblProbe, Object)

IblProbe::IblProbe(drawing::Image* radiance)
:	m_radiance(radiance)
{
}

Color4f IblProbe::sampleRadiance(const Vector4& direction) const
{
	Vector2 e = toEquirectangular(direction);

	float x = e.x / TWO_PI;
	float y = e.y / PI;
	
	int32_t w = m_radiance->getWidth();
	int32_t h = m_radiance->getHeight();
	int32_t u = (int32_t)(x * (w - 1));
	int32_t v = (int32_t)(y * (h - 1));

	Color4f cl;
	m_radiance->getPixel(u, v, cl);
	return cl;
}

void IblProbe::apply(const drawing::IImageFilter* imageFilter)
{
	m_radiance->apply(imageFilter);
}

	}
}
