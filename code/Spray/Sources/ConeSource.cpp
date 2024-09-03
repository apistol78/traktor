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
#include "Spray/Sources/ConeSource.h"

namespace traktor::spray
{
	namespace
	{

const resource::Id< render::Shader > c_shaderConeSource(L"{1BF7210A-0A23-E041-988D-44AADC38D06E}");

static render::Handle s_handleSourcePosition(L"Spray_SourcePosition");
static render::Handle s_handleSourceNormal(L"Spray_SourceNormal");
static render::Handle s_handleSourceAngles(L"Spray_SourceAngles");
static render::Handle s_handleSourceVelocity(L"Spray_SourceVelocity");
static render::Handle s_handleSourceInheritVelocity(L"Spray_SourceInheritVelocity");
static render::Handle s_handleSourceOrientation(L"Spray_SourceOrientation");
static render::Handle s_handleSourceAngularVelocity(L"Spray_SourceAngularVelocity");
static render::Handle s_handleSourceAge(L"Spray_SourceAge");
static render::Handle s_handleSourceMass(L"Spray_SourceMass");
static render::Handle s_handleSourceSize(L"Spray_SourceSize");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.ConeSource", ConeSource, Source)

ConeSource::ConeSource(
	float constantRate,
	float velocityRate,
	const Vector4& position,
	const Vector4& normal,
	float angle1,
	float angle2,
	const Range< float >& velocity,
	const Range< float >& inheritVelocity,
	const Range< float >& orientation,
	const Range< float >& angularVelocity,
	const Range< float >& age,
	const Range< float >& mass,
	const Range< float >& size
)
:	Source(constantRate, velocityRate)
,	m_position(position.xyz1())
,	m_normal(normal.xyz0())
,	m_angle1s(sinf(angle1))
,	m_angle2s(sinf(angle2))
,	m_velocity(velocity)
,	m_inheritVelocity(inheritVelocity)
,	m_orientation(orientation)
,	m_angularVelocity(angularVelocity)
,	m_age(age)
,	m_mass(mass)
,	m_size(size)
{
}

resource::Id< render::Shader > ConeSource::getShader() const
{
	return c_shaderConeSource;
}

void ConeSource::setShaderParameters(render::ProgramParameters* pp) const
{
	pp->setVectorParameter(s_handleSourcePosition, m_position);
	pp->setVectorParameter(s_handleSourceNormal, m_normal);
	pp->setVectorParameter(s_handleSourceAngles, Vector4(m_angle1s, m_angle2s, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceVelocity, Vector4(m_velocity.min, m_velocity.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceInheritVelocity, Vector4(m_inheritVelocity.min, m_inheritVelocity.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceOrientation, Vector4(m_orientation.min, m_orientation.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceAngularVelocity, Vector4(m_angularVelocity.min, m_angularVelocity.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceAge, Vector4(m_age.min, m_age.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceMass, Vector4(m_mass.min, m_mass.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceSize, Vector4(m_size.min, m_size.max, 0.0f, 0.0f));
}

void ConeSource::emit(
	Context& context,
	const Transform& transform,
	const Vector4& deltaMotion,
	uint32_t emitCount,
	EmitterInstanceCPU& emitterInstance
) const
{
	const Vector4 position = transform * m_position;
	const Vector4 normal = transform * m_normal;

	const Scalar dT(context.deltaTime);

	const Vector4 deltaVelocity = -deltaMotion / dT;

	const Vector4 tx = transform.axisX() * m_angle1s;
	const Vector4 tz = transform.axisZ() * m_angle2s;

	Point* point = emitterInstance.addPoints(emitCount);
	while (emitCount-- > 0)
	{
		const float phi = context.random.nextFloat() * 2.0f * PI;

		const Scalar gamma(context.random.nextFloat());
		const Scalar beta(context.random.nextFloat());

		const Scalar x(traktor::sinf(phi + HALF_PI));
		const Scalar z(traktor::sinf(phi));

		const Vector4 ax = tx * x;
		const Vector4 az = tz * z;

		const Vector4 extent = ax + az;
		const Vector4 direction = (normal + extent * gamma).normalized();

		point->velocity = direction * Scalar(m_velocity.random(context.random)) + deltaVelocity * Scalar(m_inheritVelocity.random(context.random));
		point->position = position + point->velocity * beta * dT;
		point->orientation = m_orientation.random(context.random);
		point->angularVelocity = m_angularVelocity.random(context.random);
		point->age = 0.0f;
		point->maxAge = m_age.random(context.random);
		point->inverseMass = 1.0f / m_mass.random(context.random);
		point->size = m_size.random(context.random);
		point->random = context.random.nextFloat();

		++point;
	}
}

}
