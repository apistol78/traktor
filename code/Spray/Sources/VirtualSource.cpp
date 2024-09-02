/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Types.h"
#include "Spray/Sources/VirtualSource.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.VirtualSource", VirtualSource, Source)

VirtualSource::VirtualSource(
	float constantRate,
	float velocityRate,
	const Range< float >& velocity,
	const Range< float >& orientation,
	const Range< float >& angularVelocity,
	const Range< float >& age,
	const Range< float >& mass,
	const Range< float >& size
)
:	Source(constantRate, velocityRate)
,	m_velocity(velocity)
,	m_orientation(orientation)
,	m_angularVelocity(angularVelocity)
,	m_age(age)
,	m_mass(mass)
,	m_size(size)
{
}

void VirtualSource::emit(
	Context& context,
	const Transform& transform,
	const Vector4& deltaMotion,
	uint32_t emitCount,
	EmitterInstanceCPU& emitterInstance
) const
{
	if (context.virtualSourceCallback)
		context.virtualSourceCallback->virtualSourceEmit(
			this,
			context,
			transform,
			deltaMotion,
			emitCount,
			emitterInstance
		);
}

}
