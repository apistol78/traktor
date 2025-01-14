/*
 * TRAKTOR
 * Copyright (c) 2023-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Bake/SkyProbe.h"

#include "Core/Math/Matrix44.h"
#include "Core/Math/Quasirandom.h"
#include "Render/SH/SHEngine.h"
#include "Render/SH/SHFunction.h"

#include <functional>

namespace traktor::shape
{
namespace
{

class WrappedSHFunction : public render::SHFunction
{
public:
	explicit WrappedSHFunction(const std::function< Vector4(const Vector4&) >& fn)
		: m_fn(fn)
	{
	}

	virtual Vector4 evaluate(const Polar& direction) const override final
	{
		return m_fn(direction.toUnitCartesian());
	}

private:
	std::function< Vector4(const Vector4&) > m_fn;
};

Vector4 lambertianDirection(const Vector2& uv, const Vector4& direction)
{
	// Calculate random direction, with Gaussian probability distribution.
	const float sin2_theta = uv.x;
	const float cos2_theta = 1.0f - sin2_theta;
	const float sin_theta = std::sqrt(sin2_theta);
	const float cos_theta = std::sqrt(cos2_theta);
	const float orientation = uv.y * TWO_PI;
	const Vector4 dir(sin_theta * std::cos(orientation), sin_theta * std::sin(orientation), cos_theta, 0.0f);

	Vector4 u, v;
	orthogonalFrame(direction, u, v);
	return (Matrix44(u, v, direction, Vector4::zero()) * dir).xyz0().normalized();
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SkyProbe", 0, SkyProbe, IProbe)

SkyProbe::SkyProbe(const Color4f& skyOverHorizon, const Color4f& skyUnderHorizon, float intensity_, float saturation_)
{
	const Scalar intensity(intensity_);
	const Scalar saturation(saturation_);

	WrappedSHFunction shFunction([&](const Vector4& unit) -> Vector4 {
		Color4f cl(0.0f, 0.0f, 0.0f, 0.0f);
		const Vector4 rd = unit;

		// Sample over hemisphere.
		for (int32_t i = 0; i < 1000; ++i)
		{
			const Vector2 uv = Quasirandom::hammersley(i, 1000);

			// const Vector4 direction = Quasirandom::uniformHemiSphere(uv, rd);
			// const Scalar probability = 1.0_simd;

			const Vector4 direction = lambertianDirection(uv, rd);
			const Scalar probability = 0.78532_simd;

			Vector4 col = Vector4(skyOverHorizon.linear()) - max(rd.y(), 0.01_simd) * max(rd.y(), 0.01_simd) * 0.5_simd;
			col = lerp(col, skyUnderHorizon.linear(), power(1.0_simd - max(rd.y(), 0.0_simd), 6.0_simd));

			const Scalar cosPhi = dot3(direction, rd);
			cl += Color4f(col * cosPhi / probability);
		}

		// Apply saturation.
		const Scalar bw = dot3(cl, Vector4(1.0f, 1.0f, 1.0f)) / 3.0_simd;
		cl = Color4f(lerp(Vector4(bw, bw, bw, 0.0f), cl, saturation));

		return (cl * intensity * 3.0_simd) / 1000.0_simd;
	});

	render::SHEngine shEngine(3);
	shEngine.generateSamplePoints(10000);
	shEngine.generateCoefficients(&shFunction, true, m_shCoeffs);
}

Color4f SkyProbe::sampleRadiance(const Vector4& direction) const
{
	return Color4f(m_shCoeffs.evaluate3(Polar::fromUnitCartesian(direction)));
}

void SkyProbe::serialize(ISerializer& s)
{
}

}
