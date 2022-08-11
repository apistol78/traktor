#include "Spray/Modifiers/VortexModifier.h"

namespace traktor
{
	namespace spray
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
	Vector4 axis = m_world ? m_axis : transform * m_axis;
	Vector4 center = m_world ? transform.translation() : Vector4::origo();

	for (size_t i = first; i < last; ++i)
	{
		Vector4 pc = points[i].position - center;

		// Project onto plane.
		Scalar d = dot3(pc, axis);
		pc -= axis * d;

		// Calculate tangent vector.
		Scalar distance = pc.length();
		Vector4 n = pc / distance;
		Vector4 t = cross(axis, n).normalized();

		// Adjust velocity from this tangent.
		points[i].velocity += (
			t * m_tangentForce +
			n * (m_normalConstantForce + (distance - m_normalDistance) * m_normalDistanceForce)
		) * Scalar(points[i].inverseMass) * deltaTime;
	}
}

	}
}
