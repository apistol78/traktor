/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Bake/SkyProbe.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SkyProbe", 0, SkyProbe, IProbe)

SkyProbe::SkyProbe(const Vector4& sunDirection)
:   m_sunDirection(sunDirection)
{
}

Color4f SkyProbe::sampleRadiance(const Vector4& direction) const
{
    const Scalar sundot = clamp(dot3(direction, m_sunDirection), 0.0_simd, 1.0_simd);

    Vector4 col = Vector4(0.2f, 0.5f, 0.85f, 0.0f) * 1.1_simd - max(direction.y(), 0.01_simd) * max(direction.y(), 0.01_simd) * 0.5_simd;
    col = lerp(col, 0.85_simd * Vector4(0.7f, 0.75f, 0.85f), power(1.0_simd - max(direction.y(), 0.0_simd), 6.0_simd));

    col += 0.25_simd * Vector4(1.0f, 0.7f, 0.4f) * power(sundot, 5.0_simd);
    col += 0.25_simd * Vector4(1.0f, 0.8f, 0.6f) * power(sundot, 64.0_simd);
    col += 0.20_simd * Vector4(1.0f, 0.8f, 0.6f) * power(sundot, 512.0_simd);

    col += clamp((0.1_simd - direction.y()) * 10.0_simd, 0.0_simd, 1.0_simd) * Vector4(0.0f, 0.1f, 0.2f, 0.0f);
    //col += sunColor * pow(sundot, 8.0f);

    return Color4f(col);
}

void SkyProbe::serialize(ISerializer& s)
{
}

}
