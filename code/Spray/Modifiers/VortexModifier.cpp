#include "Spray/Modifiers/VortexModifier.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.VortexModifier", VortexModifier, Modifier)

VortexModifier::VortexModifier()
:	m_axis(0.0f, 1.0f, 0.0f, 0.0f)
,	m_tangentForce(0.0f)
,	m_normalConstantForce(0.0f)
,	m_normalDistance(0.0f)
,	m_normalDistanceForce(0.0f)
{
}

void VortexModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	Vector4 center = transform.translation();
	for (size_t i = first; i < last; ++i)
	{
		Vector4 pc = points[i].position - center;

		// Project onto plane.
		Scalar d = dot3(pc, m_axis);
		pc -= m_axis * d;

		// Calculate tangent vector.
		Scalar distance = pc.length();
		Vector4 n = pc / distance;
		Vector4 t = cross(m_axis, n).normalized();

		// Adjust velocity from this tangent.
		points[i].velocity += (
			t * m_tangentForce +
			n * (m_normalConstantForce + (distance - m_normalDistance) * m_normalDistanceForce)
		) * Scalar(points[i].inverseMass) * deltaTime;
	}
}

bool VortexModifier::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"axis", m_axis);
	s >> Member< Scalar >(L"tangentForce", m_tangentForce);
	s >> Member< Scalar >(L"normalConstantForce", m_normalConstantForce);
	s >> Member< Scalar >(L"normalDistance", m_normalDistance);
	s >> Member< Scalar >(L"normalDistanceForce", m_normalDistanceForce);
	return true;
}

	}
}
