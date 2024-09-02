/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/EmitterInstanceCPU.h"
#include "Spray/Types.h"
#include "Spray/Sources/LineSource.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.LineSource", LineSource, Source)

LineSource::LineSource(
	float constantRate,
	float velocityRate,
	const Vector4& startPosition,
	const Vector4& endPosition,
	int32_t segments,
	const Range< float >& velocity,
	const Range< float >& orientation,
	const Range< float >& angularVelocity,
	const Range< float >& age,
	const Range< float >& mass,
	const Range< float >& size
)
:	Source(constantRate, velocityRate)
,	m_startPosition(startPosition.xyz1())
,	m_endPosition(endPosition.xyz1())
,	m_segments(segments)
,	m_velocity(velocity)
,	m_orientation(orientation)
,	m_angularVelocity(angularVelocity)
,	m_age(age)
,	m_mass(mass)
,	m_size(size)
{
}

void LineSource::emit(
	Context& context,
	const Transform& transform,
	const Vector4& deltaMotion,
	uint32_t emitCount,
	EmitterInstanceCPU& emitterInstance
) const
{
	const Vector4 startPosition = transform * m_startPosition;
	const Vector4 endPosition = transform * m_endPosition;
	const Vector4 direction = (endPosition - startPosition).normalized();

	const int32_t npoints = 2 + m_segments;

	Point* point = emitterInstance.addPoints(npoints);

	for (int32_t i = 0; i < npoints; ++i)
	{
		const Vector4 position = lerp(startPosition, endPosition, Scalar(float(i) / (npoints - 1)));

		point->position = position;
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
