/*
 * TRAKTOR
 * Copyright (c) 2023-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Bake/SkyProbe.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SkyProbe", 0, SkyProbe, IProbe)

SkyProbe::SkyProbe(const Color4f& skyOverHorizon, const Color4f& skyUnderHorizon, float intensity)
:   m_skyOverHorizon(skyOverHorizon)
,	m_skyUnderHorizon(skyUnderHorizon)
,	m_intensity(intensity)
{
}

Color4f SkyProbe::sampleRadiance(const Vector4& direction) const
{
    const Vector4 rd = direction;

	Vector4 col = Vector4(m_skyOverHorizon.linear()) - max(rd.y(), 0.01_simd) * max(rd.y(), 0.01_simd) * 0.5_simd;
	col = lerp(col, m_skyUnderHorizon.linear(), power(1.0_simd - max(rd.y(), 0.0_simd), 6.0_simd));

    return Color4f(col) * Scalar(m_intensity);
}

void SkyProbe::serialize(ISerializer& s)
{
}

}
