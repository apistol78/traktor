/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Context/ProgramParameters.h"
#include "Spray/EmitterInstanceCPU.h"
#include "Spray/Types.h"
#include "Spray/Sources/BoxSource.h"

namespace traktor::spray
{
	namespace
	{

const resource::Id< render::Shader > c_shaderBoxSource(L"{7A15F36C-25BD-664D-8818-47A7F023127D}");

static render::Handle s_handleSourcePosition(L"Spray_SourcePosition");
static render::Handle s_handleSourceExtent(L"Spray_SourceExtent");
static render::Handle s_handleSourceVelocity(L"Spray_SourceVelocity");
static render::Handle s_handleSourceOrientation(L"Spray_SourceOrientation");
static render::Handle s_handleSourceAngularVelocity(L"Spray_SourceAngularVelocity");
static render::Handle s_handleSourceAge(L"Spray_SourceAge");
static render::Handle s_handleSourceMass(L"Spray_SourceMass");
static render::Handle s_handleSourceSize(L"Spray_SourceSize");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.BoxSource", BoxSource, Source)

BoxSource::BoxSource(
	float constantRate,
	float velocityRate,
	const Vector4& position,
	const Vector4& extent,
	const Range< float >& velocity,
	const Range< float >& orientation,
	const Range< float >& angularVelocity,
	const Range< float >& age,
	const Range< float >& mass,
	const Range< float >& size
)
:	Source(constantRate, velocityRate)
,	m_position(position.xyz1())
,	m_extent(extent.xyz0())
,	m_velocity(velocity)
,	m_orientation(orientation)
,	m_angularVelocity(angularVelocity)
,	m_age(age)
,	m_mass(mass)
,	m_size(size)
{
}

resource::Id< render::Shader > BoxSource::getShader() const
{
	return c_shaderBoxSource;
}

void BoxSource::setShaderParameters(render::ProgramParameters* pp) const
{
	pp->setVectorParameter(s_handleSourcePosition, m_position);
	pp->setVectorParameter(s_handleSourceExtent, m_extent);
	pp->setVectorParameter(s_handleSourceVelocity, Vector4(m_velocity.min, m_velocity.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceOrientation, Vector4(m_orientation.min, m_orientation.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceAngularVelocity, Vector4(m_angularVelocity.min, m_angularVelocity.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceAge, Vector4(m_age.min, m_age.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceMass, Vector4(m_mass.min, m_mass.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceSize, Vector4(m_size.min, m_size.max, 0.0f, 0.0f));
}

void BoxSource::emit(
	Context& context,
	const Transform& transform,
	const Vector4& deltaMotion,
	uint32_t emitCount,
	EmitterInstanceCPU& emitterInstance
) const
{
	const Vector4 position = transform * m_position - m_extent * 0.5_simd;

	Point* point = emitterInstance.addPoints(emitCount);

	while (emitCount-- > 0)
	{
		const Vector4 direction = context.random.nextUnit();

		point->position = position + m_extent * Vector4(context.random.nextFloat(), context.random.nextFloat(), context.random.nextFloat(), 0.0f);
		point->velocity = direction * Scalar(m_velocity.random(context.random));
		point->orientation = m_orientation.random(context.random);
		point->angularVelocity = m_angularVelocity.random(context.random);
		point->age = 0.0f;
		point->maxAge = m_age.random(context.random);
		point->inverseMass = 1.0f / (m_mass.random(context.random));
		point->size = m_size.random(context.random);
		point->random = context.random.nextFloat();

		++point;
	}
}

}
