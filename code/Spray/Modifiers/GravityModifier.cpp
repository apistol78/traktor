#include "Spray/Modifiers/GravityModifier.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.spray.GravityModifier", GravityModifier, Modifier)

GravityModifier::GravityModifier()
:	m_gravity(0.0f, 0.0f, 0.0f, 0.0f)
{
}

void GravityModifier::update(const Scalar& deltaTime, const Matrix44& transform, PointVector& points, size_t first, size_t last) const
{
	for (size_t i = first; i < last; ++i)
		points[i].velocity += m_gravity * Scalar(points[i].inverseMass) * deltaTime;
}

bool GravityModifier::serialize(Serializer& s)
{
	return s >> Member< Vector4 >(L"gravity", m_gravity);
}

	}
}
