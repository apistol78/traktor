/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Spray/EmitterInstance.h"
#include "Spray/Types.h"
#include "Spray/Sources/PointSource.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointSource", PointSource, Source)

PointSource::PointSource(
	float constantRate,
	float velocityRate,
	const Vector4& position,
	const Range< float >& velocity,
	const Range< float >& orientation,
	const Range< float >& angularVelocity,
	const Range< float >& age,
	const Range< float >& mass,
	const Range< float >& size
)
:	Source(constantRate, velocityRate)
,	m_position(position.xyz1())
,	m_velocity(velocity)
,	m_orientation(orientation)
,	m_angularVelocity(angularVelocity)
,	m_age(age)
,	m_mass(mass)
,	m_size(size)
{
}

void PointSource::emit(
	Context& context,
	const Transform& transform,
	const Vector4& deltaMotion,
	uint32_t emitCount,
	EmitterInstance& emitterInstance
) const
{
	Vector4 position = transform * m_position;

	Point* point = emitterInstance.addPoints(emitCount);

	while (emitCount-- > 0)
	{
		point->position = position - deltaMotion * Scalar(context.random.nextFloat());
		point->velocity = context.random.nextUnit().xyz0() * Scalar(m_velocity.random(context.random));
		point->orientation = m_orientation.random(context.random);
		point->angularVelocity = m_angularVelocity.random(context.random);
		point->color = Vector4::one();
		point->age = 0.0f;
		point->maxAge = m_age.random(context.random);
		point->inverseMass = 1.0f / (m_mass.random(context.random));
		point->size = m_size.random(context.random);
		point->random = context.random.nextFloat();
		
		++point;
	}
}

	}
}
