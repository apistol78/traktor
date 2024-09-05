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
#include "Spray/Sources/QuadSource.h"

namespace traktor::spray
{
	namespace
	{

const resource::Id< render::Shader > c_shaderQuadSource(L"{97043C88-124A-D14F-8B42-CF975518E081}");

static render::Handle s_handleSourcePosition(L"Spray_SourcePosition");
static render::Handle s_handleSourceNormal(L"Spray_SourceNormal");
static render::Handle s_handleSourceAxis1(L"Spray_SourceAxis1");
static render::Handle s_handleSourceAxis2(L"Spray_SourceAxis2");
static render::Handle s_handleSourceVelocity(L"Spray_SourceVelocity");
static render::Handle s_handleSourceOrientation(L"Spray_SourceOrientation");
static render::Handle s_handleSourceAngularVelocity(L"Spray_SourceAngularVelocity");
static render::Handle s_handleSourceAge(L"Spray_SourceAge");
static render::Handle s_handleSourceMass(L"Spray_SourceMass");
static render::Handle s_handleSourceSize(L"Spray_SourceSize");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.QuadSource", QuadSource, Source)

QuadSource::QuadSource(
	float constantRate,
	float velocityRate,
	const Vector4& center,
	const Vector4& axis1,
	const Vector4& axis2,
	const Vector4& normal,
	const Range< float >& velocity,
	const Range< float >& orientation,
	const Range< float >& angularVelocity,
	const Range< float >& age,
	const Range< float >& mass,
	const Range< float >& size
)
:	Source(constantRate, velocityRate)
,	m_center(center.xyz1())
,	m_axis1(axis1.xyz0())
,	m_axis2(axis2.xyz0())
,	m_normal(normal.xyz0())
,	m_velocity(velocity)
,	m_orientation(orientation)
,	m_angularVelocity(angularVelocity)
,	m_age(age)
,	m_mass(mass)
,	m_size(size)
{
}

resource::Id< render::Shader > QuadSource::getShader() const
{
	return c_shaderQuadSource;
}

void QuadSource::setShaderParameters(render::ProgramParameters* pp) const
{
	pp->setVectorParameter(s_handleSourcePosition, m_center);
	pp->setVectorParameter(s_handleSourceNormal, m_normal);
	pp->setVectorParameter(s_handleSourceAxis1, m_axis1);
	pp->setVectorParameter(s_handleSourceAxis2, m_axis2);
	pp->setVectorParameter(s_handleSourceVelocity, Vector4(m_velocity.min, m_velocity.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceOrientation, Vector4(m_orientation.min, m_orientation.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceAngularVelocity, Vector4(m_angularVelocity.min, m_angularVelocity.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceAge, Vector4(m_age.min, m_age.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceMass, Vector4(m_mass.min, m_mass.max, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleSourceSize, Vector4(m_size.min, m_size.max, 0.0f, 0.0f));
}

void QuadSource::emit(
	Context& context,
	const Transform& transform,
	const Vector4& deltaMotion,
	uint32_t emitCount,
	EmitterInstanceCPU& emitterInstance
) const
{
	const Vector4 center = transform * m_center;
	const Vector4 axis1 = transform * m_axis1;
	const Vector4 axis2 = transform * m_axis2;
	const Vector4 normal = transform * m_normal;

	Point* point = emitterInstance.addPoints(emitCount);

	while (emitCount-- > 0)
	{
		const Scalar u = Scalar(float(context.random.nextDouble()) * 2.0f - 1.0f);
		const Scalar v = Scalar(float(context.random.nextDouble()) * 2.0f - 1.0f);

		point->position = center + u * axis1 + v * axis2;
		point->velocity = normal * Scalar(m_velocity.random(context.random));
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
