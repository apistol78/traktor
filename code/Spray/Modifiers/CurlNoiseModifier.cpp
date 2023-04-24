/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Modifiers/CurlNoiseModifier.h"

namespace traktor::spray
{
	namespace
	{
	
float snoise(const Vector4& x)
{
	return 0.0f;
}

Vector4 snoiseVector4(const Vector4& x)
{
	const float s = snoise(x);
	const float s1 = snoise(Vector4(x.y() - 19.1f, x.z() - 33.4f, x.x() + 47.2f));
	const float s2 = snoise(Vector4(x.z() + 74.2f, x.x() - 124.5f, x.y() + 99.4f));
	return Vector4(s, s1, s2);
}

Vector4 curlNoise(Vector4 p)
{
	const Scalar e = 0.1_simd;

	Vector4 dx = Vector4(e, 0.0f, 0.0f);
	Vector4 dy = Vector4(0.0f, e, 0.0f);
	Vector4 dz = Vector4(0.0f, 0.0f, e);

	Vector4 p_x0 = snoiseVector4(p - dx);
	Vector4 p_x1 = snoiseVector4(p + dx);
	Vector4 p_y0 = snoiseVector4(p - dy);
	Vector4 p_y1 = snoiseVector4(p + dy);
	Vector4 p_z0 = snoiseVector4(p - dz);
	Vector4 p_z1 = snoiseVector4(p + dz);

	float x = p_y1.z() - p_y0.z() - p_z1.y() + p_z0.y();
	float y = p_z1.x() - p_z0.x() - p_x1.z() + p_x0.z();
	float z = p_x1.y() - p_x0.y() - p_y1.x() + p_y0.x();

	const Scalar divisor = 1.0_simd / (2.0_simd * e);
	return (Vector4(x, y, z) * divisor).normalized();

}
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.CurlNoiseModifier", CurlNoiseModifier, Modifier)

CurlNoiseModifier::CurlNoiseModifier(float factor)
:	m_factor(factor)
{
}

void CurlNoiseModifier::update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const
{
	for (size_t i = first; i < last; ++i)
	{
		const Vector4 r = curlNoise(points[i].position);
		points[i].velocity += (r * m_factor * deltaTime) * Scalar(points[i].inverseMass);
	}
}

}
