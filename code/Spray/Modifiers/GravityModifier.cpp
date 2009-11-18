#include "Spray/Modifiers/GravityModifier.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.GravityModifier", 0, GravityModifier, Modifier)

GravityModifier::GravityModifier()
:	m_gravity(0.0f, 0.0f, 0.0f, 0.0f)
{
}

void GravityModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	for (size_t i = first; i < last; ++i)
		points[i].velocity += m_gravity * Scalar(points[i].inverseMass) * deltaTime;
}

bool GravityModifier::serialize(ISerializer& s)
{
	return s >> Member< Vector4 >(L"gravity", m_gravity);
}

	}
}
