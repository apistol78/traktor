#include "Spray/Modifiers/IntegrateModifier.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.IntegrateModifier", IntegrateModifier, Modifier)

IntegrateModifier::IntegrateModifier()
:	m_timeScale(1.0f)
{
}

void IntegrateModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	Scalar scaledDeltaTime = deltaTime * m_timeScale;
	for (size_t i = first; i < last; ++i)
	{
		points[i].position += points[i].velocity * Scalar(points[i].inverseMass) * scaledDeltaTime;
		points[i].orientation += points[i].angularVelocity * scaledDeltaTime;
	}
}

bool IntegrateModifier::serialize(ISerializer& s)
{
	return s >> Member< Scalar >(L"timeScale", m_timeScale);
}

	}
}
