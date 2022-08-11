#include "Spray/Modifiers/GravityModifier.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.GravityModifier", GravityModifier, Modifier)

GravityModifier::GravityModifier(const Vector4& gravity, bool world)
:	m_gravity(gravity)
,	m_world(world)
{
}

void GravityModifier::update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const
{
	Vector4 gravity = (m_world ? m_gravity : transform * m_gravity) * deltaTime;
	for (size_t i = first; i < last; ++i)
		points[i].velocity += gravity * Scalar(points[i].inverseMass);
}

	}
}
