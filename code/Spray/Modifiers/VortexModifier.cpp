/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Modifiers/VortexModifier.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.VortexModifier", VortexModifier, Modifier)

VortexModifier::VortexModifier(
	const Vector4& axis,
	float tangentForce,
	float normalConstantForce,
	float normalDistance,
	float normalDistanceForce,
	bool world
)
:	m_axis(axis)
,	m_tangentForce(tangentForce)
,	m_normalConstantForce(normalConstantForce)
,	m_normalDistance(normalDistance)
,	m_normalDistanceForce(normalDistanceForce)
,	m_world(world)
{
}

void VortexModifier::update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const
{
	const Vector4 axis = m_world ? m_axis : transform * m_axis;
	const Vector4 center = m_world ? transform.translation() : Vector4::origo();

	for (size_t i = first; i < last; ++i)
	{
		Vector4 pc = points[i].position - center;

		// Project onto plane.
		const Scalar d = dot3(pc, axis);
		pc -= axis * d;

		// Calculate tangent vector.
		const Scalar distance = pc.length();
		const Vector4 n = pc / distance;
		const Vector4 t = cross(axis, n).normalized();

		// Adjust velocity from this tangent.
		points[i].velocity += (
			t * m_tangentForce +
			n * (m_normalConstantForce + (distance - m_normalDistance) * m_normalDistanceForce)
		) * Scalar(points[i].inverseMass) * deltaTime;
	}
}

}
