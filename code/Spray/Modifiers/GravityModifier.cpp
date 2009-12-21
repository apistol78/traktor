#include "Spray/Modifiers/GravityModifier.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.GravityModifier", 1, GravityModifier, Modifier)

GravityModifier::GravityModifier()
:	m_gravity(0.0f, 0.0f, 0.0f, 0.0f)
,	m_world(true)
{
}

void GravityModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	Vector4 gravity = m_world ? m_gravity : transform * m_gravity;
	for (size_t i = first; i < last; ++i)
		points[i].velocity += gravity * Scalar(points[i].inverseMass) * deltaTime;
}

bool GravityModifier::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"gravity", m_gravity);
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"world", m_world);
	return true;
}

	}
}
