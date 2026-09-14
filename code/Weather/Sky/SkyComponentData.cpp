/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Weather/Sky/SkyComponentData.h"

#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeHdr.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/ITexture.h"
#include "Render/SH/SHEngine.h"
#include "Render/SH/SHFunction.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Weather/Sky/SkyComponent.h"
#include "World/IrradianceGrid.h"

#include <cmath>

namespace traktor::weather
{
namespace
{

const resource::Id< render::Shader > c_defaultShader(Guid(L"{4CF929EB-3A8B-C340-AA0A-0C5C80625BF1}"));

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

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.weather.SkyComponentData", 8, SkyComponentData, world::IEntityComponentData)

SkyComponentData::SkyComponentData()
	: m_shader(c_defaultShader)
{
}

Ref< SkyComponent > SkyComponentData::createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;

	resource::Proxy< render::ITexture > texture;
	if (m_texture.isValid() && !m_texture.isNull())
	{
		if (!resourceManager->bind(m_texture, texture))
			return nullptr;
	}

	// Create irradiance grid from sky; project sky radiance, sun excluded since it's lit separately.
	const Scalar intensity(m_intensity);
	const Scalar saturation(m_saturation);

	WrappedSHFunction shFunction([&](const Vector4& rd) -> Vector4 {
		Vector4 col = Vector4(m_skyOverHorizon.linear()) - max(rd.y(), 0.01_simd) * max(rd.y(), 0.01_simd) * 0.5_simd;
		col = lerp(col, m_skyUnderHorizon.linear(), power(1.0_simd - max(rd.y(), 0.0_simd), 6.0_simd));

		// Apply saturation.
		const Scalar bw = dot3(col, Vector4(1.0f, 1.0f, 1.0f)) / 3.0_simd;
		col = lerp(Vector4(bw, bw, bw, 0.0f), col, saturation);

		return col * intensity;
	});

	render::SHCoeffs shCoeffs;

	render::SHEngine shEngine(3);
	shEngine.generateSamplePoints(10000);
	shEngine.generateCoefficients(&shFunction, true, shCoeffs);

	// Convolve radiance with the clamped cosine lobe into irradiance; per band factors are
	// pi, 2pi/3 and pi/4 but the shaders expect irradiance divided by pi.
	for (uint32_t i = 1; i < 4; ++i)
		shCoeffs[i] *= Scalar(2.0f / 3.0f);
	for (uint32_t i = 4; i < 9; ++i)
		shCoeffs[i] *= 0.25_simd;

	Ref< world::IrradianceGrid > irradianceGrid = world::IrradianceGrid::createSingle(renderSystem, shCoeffs);
	if (!irradianceGrid)
		return nullptr;

	Ref< SkyComponent > skyComponent = new SkyComponent(
		*this,
		irradianceGrid,
		shader,
		texture);
	skyComponent->create(resourceManager, renderSystem);
	return skyComponent;
}

int32_t SkyComponentData::getOrdinal() const
{
	return 0;
}

void SkyComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void SkyComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader, AttributePrivate());

	if (s.getVersion< SkyComponentData >() >= 2)
		s >> resource::Member< render::ITexture >(L"texture", m_texture);

	if (s.getVersion< SkyComponentData >() < 1)
	{
		Vector4 dummy;
		s >> Member< Vector4 >(L"sunDirection", dummy);
	}

	if (s.getVersion< SkyComponentData >() < 3)
	{
		float dummy;
		s >> Member< float >(L"offset", dummy);
	}

	if (s.getVersion< SkyComponentData >() >= 4)
		s >> Member< float >(L"intensity", m_intensity, AttributeRange(0.0f) | AttributeUnit(UnitType::Percent));

	if (s.getVersion< SkyComponentData >() >= 8)
		s >> Member< float >(L"saturation", m_saturation, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));

	if (s.getVersion< SkyComponentData >() >= 7)
	{
		s >> Member< Color4f >(L"skyOverHorizon", m_skyOverHorizon, AttributeHdr());
		s >> Member< Color4f >(L"skyUnderHorizon", m_skyUnderHorizon, AttributeHdr());

		s >> Member< bool >(L"clouds", m_clouds);
		s >> Member< Color4f >(L"cloudAmbientTop", m_cloudAmbientTop, AttributeHdr());
		s >> Member< Color4f >(L"cloudAmbientBottom", m_cloudAmbientBottom, AttributeHdr());
	}
	else
	{
		if (s.getVersion< SkyComponentData >() >= 5)
			s >> Member< bool >(L"clouds", m_clouds);

		if (s.getVersion< SkyComponentData >() == 6)
		{
			s >> Member< Color4f >(L"overHorizon", m_skyOverHorizon);
			s >> Member< Color4f >(L"underHorizon", m_skyUnderHorizon);
		}
	}
}

}
