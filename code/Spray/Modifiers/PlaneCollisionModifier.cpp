#include "Spray/Modifiers/PlaneCollisionModifier.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.PlaneCollisionModifier", PlaneCollisionModifier, Modifier)

PlaneCollisionModifier::PlaneCollisionModifier()
:	m_plane(Vector4(0.0f, 1.0f, 0.0f), Scalar(0.0f))
,	m_restitution(1.0f)
{
}

void PlaneCollisionModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	for (size_t i = first; i < last; ++i)
	{
		Scalar rv = dot3(m_plane.normal(), points[i].velocity);
		if (rv >= 0.0f)
			continue;

		Scalar rd = m_plane.distance(points[i].position);
		if (rd >= points[i].size)
			continue;

		points[i].velocity = -reflect(points[i].velocity, m_plane.normal()) * m_restitution;
	}
}

bool PlaneCollisionModifier::serialize(ISerializer& s)
{
	// @fixme Plane
	s >> Member< Scalar >(L"restitution", m_restitution);
	return true;
}

	}
}
