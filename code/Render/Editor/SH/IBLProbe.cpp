/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Render/Editor/SH/IBLProbe.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"IBLProbe", IBLProbe, SHFunction)

IBLProbe::IBLProbe(drawing::Image* image)
:	m_image(image)
{
}

Vector4 IBLProbe::evaluate(float phi, float theta, const Vector4& unit) const
{
	float T_MATH_ALIGN16 u[4];
	unit.storeAligned(u);

	float r = float((1.0f / PI) * acos(u[2]) / sqrt(u[0] * u[0] + u[1] * u[1]));

	int x = int(m_image->getWidth() * (u[0] * r * 0.5f + 0.5f));
	int y = int(m_image->getHeight() * (-u[1] * r * 0.5f + 0.5f));

	if (x < 0 || y < 0)
		return Vector4::zero();

	Color4f color;
	if (!m_image->getPixel(x, y, color))
		return Vector4::zero();

	return color;
}

	}
}
