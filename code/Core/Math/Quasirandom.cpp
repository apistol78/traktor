/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Quasirandom.h"
#include "Core/Math/Random.h"

namespace traktor
{

Vector2 Quasirandom::hammersley(uint32_t i, uint32_t numSamples)
{
	uint32_t b = i;
	b = (b << 16u) | (b >> 16u);
	b = ((b & 0x55555555u) << 1u) | ((b & 0xAAAAAAAAu) >> 1u);
	b = ((b & 0x33333333u) << 2u) | ((b & 0xCCCCCCCCu) >> 2u);
	b = ((b & 0x0F0F0F0Fu) << 4u) | ((b & 0xF0F0F0F0u) >> 4u);
	b = ((b & 0x00FF00FFu) << 8u) | ((b & 0xFF00FF00u) >> 8u);
	const float radicalInverseVDC = (float)(b * 2.3283064365386963e-10);
    return Vector2((float)i / numSamples, radicalInverseVDC);
}

Vector2 Quasirandom::hammersley(uint32_t i, uint32_t numSamples, Random& rnd)
{
	const float rx = rnd.nextFloat() * 0.1f;
	const float ry = rnd.nextFloat() * 0.1f;
    return hammersley(i, numSamples) * 0.9f + Vector2(rx, ry);
}

Vector4 Quasirandom::uniformSphere(const Vector2& uv)
{
	const float z = 2.0f * uv.x - 1.0f;
	const float t = 2.0f * PI * uv.y;
	const float w = std::sqrt(1.0f - z * z);
	const float x = w * std::cos(t);
	const float y = w * std::sin(t);
	return Vector4(x, y, z, 0.0f);
}

Vector4 Quasirandom::uniformHemiSphere(const Vector2& uv, const Vector4& direction)
{
	const Vector4 v = uniformSphere(uv);
	return (dot3(v, direction) >= 0.0_simd) ? v : -v;
}

Vector4 Quasirandom::uniformCone(const Vector2& uv, const Vector4& direction, float radius)
{
	const float phi = (uv.x * 2.0f - 1.0f) * PI;
	const float theta = uv.y * radius;

	const Scalar sinPhi(std::sin(phi));
	const Scalar cosPhi(std::cos(phi));
	const Scalar sinTheta(std::sin(theta));
	const Scalar cosTheta(std::cos(theta));

	Vector4 u, v;
	orthogonalFrame(direction, u, v);

	const Vector4 r = sinTheta * (cosPhi * u + sinPhi * v) + cosTheta * direction;
	return r.xyz0().normalized();
}

}
