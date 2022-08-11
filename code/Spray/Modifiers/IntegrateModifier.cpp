#include "Spray/Modifiers/IntegrateModifier.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.IntegrateModifier", IntegrateModifier, Modifier)

IntegrateModifier::IntegrateModifier(float timeScale, bool linear, bool angular)
:	m_timeScale(timeScale)
,	m_linear(linear)
,	m_angular(angular)
{
}

void IntegrateModifier::update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const
{
	Scalar scaledDeltaTime = deltaTime * m_timeScale;
	if (m_linear && m_angular)
	{
		for (size_t i = first; i < last; ++i)
		{
			points[i].position += points[i].velocity * Scalar(points[i].inverseMass) * scaledDeltaTime;
			points[i].orientation += points[i].angularVelocity * scaledDeltaTime;
		}
	}
	else if (m_linear)
	{
		for (size_t i = first; i < last; ++i)
			points[i].position += points[i].velocity * Scalar(points[i].inverseMass) * scaledDeltaTime;
	}
	else if (m_angular)
	{
		for (size_t i = first; i < last; ++i)
			points[i].orientation += points[i].angularVelocity * scaledDeltaTime;
	}
}

	}
}
