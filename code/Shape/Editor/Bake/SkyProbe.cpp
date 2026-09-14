/*
 * TRAKTOR
 * Copyright (c) 2023-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Bake/SkyProbe.h"

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

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SkyProbe", 0, SkyProbe, IProbe)

SkyProbe::SkyProbe(const Color4f& skyOverHorizon, const Color4f& skyUnderHorizon, float intensity_, float saturation_)
{
	const Scalar intensity(intensity_);
	const Scalar saturation(saturation_);

	// Project sky radiance; the tracer sample radiance of escaping rays so no convolution.
	WrappedSHFunction shFunction([&](const Vector4& rd) -> Vector4 {
		Vector4 col = Vector4(skyOverHorizon.linear()) - max(rd.y(), 0.01_simd) * max(rd.y(), 0.01_simd) * 0.5_simd;
		col = lerp(col, skyUnderHorizon.linear(), power(1.0_simd - max(rd.y(), 0.0_simd), 6.0_simd));

		// Apply saturation.
		const Scalar bw = dot3(col, Vector4(1.0f, 1.0f, 1.0f)) / 3.0_simd;
		col = lerp(Vector4(bw, bw, bw, 0.0f), col, saturation);

		return col * intensity;
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
